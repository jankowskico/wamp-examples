#!/usr/bin/php
<?php require __DIR__.'/../vendor/autoload.php';

const DEFAULT_ENV = ".env.service";

if (file_exists(DEFAULT_ENV)) {
    $dotenv = \Dotenv\Dotenv::createImmutable('.', DEFAULT_ENV);
    $dotenv->load();
}

if (
    empty($_ENV['WAMP_DEBUG'])
    || empty($_ENV['WAMP_HOST'])
    || empty($_ENV['WAMP_PORT'])
    || empty($_ENV['WAMP_AUTHID'])
    || empty($_ENV['WAMP_SECRET'])
    || empty($_ENV['WAMP_REALM'])
) {
    exit;
}

if ($_ENV['WAMP_DEBUG'] != "true") {
    \Thruway\Logging\Logger::set(new \Psr\Log\NullLogger());
}

$loop = \React\EventLoop\Factory::create();

$client = new \Thruway\Peer\Client($_ENV['WAMP_REALM'], $loop);
$client->setAuthId($_ENV['WAMP_AUTHID']);
$client->addClientAuthenticator(
    new \Thruway\Authentication\ClientWampCraAuthenticator(
        $_ENV['WAMP_AUTHID'],
        $_ENV['WAMP_SECRET']
    )
);
$client->addTransportProvider(
    new \Thruway\Transport\PawlTransportProvider(
        'wss://'.$_ENV['WAMP_HOST'].':'.$_ENV['WAMP_PORT']
    )
);

$client->on('open', function (\Thruway\ClientSession $session) use ($loop) {

    $loop->addPeriodicTimer(1, function () use ($session) {
        if ($session->getState() == \Thruway\ClientSession::STATE_UP) {
            $message = [
                'This is php first argument.',
                'This is php second argument.'
            ];
            $session->publish('test', $message, [], ["acknowledge" => true])->then(
                function () use ($message) {
                    echo 'Sent: '.json_encode($message)."\n";
                },
                function ($error) {
                    echo "Call Error: {$error}\n";
                }
            );
        }
    });

});

$client->start();
