#include <dotenv/dotenv.h>
#include <autobahn/autobahn.hpp>
#include <autobahn/wamp_websocketpp_websocket_transport.hpp>

#define DEFAULT_ENV ".env.service"

class auth_wamp_session :
    public autobahn::wamp_session
{
public:
    boost::promise <autobahn::wamp_authenticate> future_challenge;
    std::string m_secret;

    auth_wamp_session(
        boost::asio::io_service& io,
        bool debug_enabled,
        const std::string& secret
    ): autobahn::wamp_session(io, debug_enabled), m_secret(secret) {}

    boost::future <autobahn::wamp_authenticate> on_challenge(
        const autobahn::wamp_challenge& challenge
    ) {
        std::string signature = compute_wcs(m_secret, challenge.challenge());
        future_challenge.set_value(autobahn::wamp_authenticate(signature));
        return future_challenge.get_future();
    }
};

void publish(
    const boost::system::error_code &,
    boost::asio::deadline_timer * timer,
    std::shared_ptr <auth_wamp_session> session,
    boost::asio::io_service * io
) {
    timer->expires_from_now(
        boost::posix_time::seconds(1)
    );

    boost::future <void> future_publish;
    boost::future <void> future_leave;
    boost::future <void> future_stop;

    std::string message[] = {
        "This is cpp first argument.",
        "This is cpp second argument."
    };

    std::cout << "Sent: [";
    for (
        int i = 0,
        n = std::distance(
            std::begin(message),
            std::end(message)
        );
        i < n;
    ) {
        std::cout << "\"" << message[i] << "\"";
        if (i++ < n - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    future_publish = session->publish("test", message).then([&](
        boost::future <void> published
    ) {
        try {
            published.get();
        } catch (const std::exception& exception) {
            std::cerr << exception.what() << std::endl;
            future_leave = session->leave().then([&](
                boost::future<std::string> reason
            ) {
                try {
                    reason.get();
                } catch (const std::exception& exception) {
                    std::cerr << exception.what() << std::endl;
                }
                future_stop = session->stop().then([&](
                    boost::future<void> stopped
                ) {
                    try {
                        stopped.get();
                    } catch (const std::exception& exception) {
                        std::cerr << exception.what() << std::endl;
                    }
                    io->stop();
                    return;
                });
            });
        }
    });

    timer->async_wait(
        boost::bind(
            publish,
            boost::asio::placeholders::error,
            timer,
            session,
            io
        )
    );
}

int main(int argc, char** argv)
{
    try {
        if (std::ifstream(DEFAULT_ENV).good()) {
            dotenv::init(DEFAULT_ENV);
        }

        if (
            std::getenv("WAMP_DEBUG") == NULL
            || std::getenv("WAMP_HOST") == NULL
            || std::getenv("WAMP_PORT") == NULL
            || std::getenv("WAMP_AUTHID") == NULL
            || std::getenv("WAMP_SECRET") == NULL
            || std::getenv("WAMP_REALM") == NULL
        ) {
            return 1;
        }

        const bool DEBUG(strcmp(std::getenv("WAMP_DEBUG"), "true") == 0);

        boost::asio::io_service io;
        boost::asio::deadline_timer timer(io, boost::posix_time::seconds(1));
        websocketpp::client <websocketpp::config::asio_tls_client> wssclient;

        boost::future <void> future_connect;
        boost::future <void> future_start;
        boost::future <void> future_join;
        boost::future <void> future_stop;

        wssclient.init_asio(&io);
        wssclient.set_tls_init_handler([&](websocketpp::connection_hdl) {
            return websocketpp::lib::make_shared
                <boost::asio::ssl::context> (boost::asio::ssl::context::tlsv13);
        });

        auto transport = std::make_shared
            <autobahn::wamp_websocketpp_websocket_transport
                <websocketpp::config::asio_tls_client>
            > (
                wssclient,
                std::string("wss://")
                    + std::getenv("WAMP_HOST")
                    + std::string(":")
                    + std::getenv("WAMP_PORT"),
                DEBUG
            );

        auto session = std::make_shared
            <auth_wamp_session> (io, DEBUG, std::getenv("WAMP_SECRET"));

        transport->attach(
            std::static_pointer_cast <autobahn::wamp_transport_handler> (session)
        );

        future_connect = transport->connect(

        ).then([&](boost::future <void> connected) {
            try {
                connected.get();
            } catch (const std::exception& exception) {
                std::cerr << exception.what() << std::endl;
                io.stop();
                return;
            }

            future_start = session->start(

            ).then([&](boost::future <void> started) {
                try {
                    started.get();
                } catch (const std::exception& exception) {
                    std::cerr << exception.what() << std::endl;
                    io.stop();
                    return;
                }

                future_join = session->join(
                    std::getenv("WAMP_REALM"),
                    {"wampcra"},
                    std::getenv("WAMP_AUTHID")
                ).then([&](boost::future <uint64_t> joined) {
                    try {
                        joined.get();
                    } catch (const std::exception& exception) {
                        std::cerr << exception.what() << std::endl;
                        future_stop = session->stop().then([&](
                            boost::future<void> stopped
                        ) {
                            try {
                                stopped.get();
                            } catch (const std::exception& exception) {
                                std::cerr << exception.what() << std::endl;
                            }
                            io.stop();
                            return;
                        });
                    }

                    timer.async_wait(
                        boost::bind(
                            publish,
                            boost::asio::placeholders::error,
                            &timer,
                            session,
                            &io
                        )
                    );
                });
            });
        });

        io.run();

        transport->detach();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return 1;
    }

    return 0;
}
