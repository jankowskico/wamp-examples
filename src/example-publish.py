#!venv/bin/python
import os, dotenv, autobahn.asyncio.component, asyncio

DEFAULT_ENV = '.env.service';

if os.path.exists(DEFAULT_ENV):
    dotenv.load_dotenv(DEFAULT_ENV)

if (
    os.environ.get('WAMP_DEBUG') is None
    or os.environ.get('WAMP_HOST') is None
    or os.environ.get('WAMP_PORT') is None
    or os.environ.get('WAMP_AUTHID') is None
    or os.environ.get('WAMP_SECRET') is None
    or os.environ.get('WAMP_REALM') is None
): exit()

component = autobahn.asyncio.component.Component(
    transports='wss://'
    + os.environ.get('WAMP_HOST') + ':' + os.environ.get('WAMP_PORT'),
    realm=os.environ.get('WAMP_REALM'),
    authentication={
        'wampcra': {
            'authid': os.environ.get('WAMP_AUTHID'),
            'secret': os.environ.get('WAMP_SECRET'),
        }
    },
)


@component.on_join
async def joined(session, details):
    while True:
        message = [
            'This is py first argument.',
            'This is py second argument.'
        ]
        session.publish('test', *message)
        print('Sent:', message)
        await asyncio.sleep(1)


if __name__ == '__main__':
    autobahn.asyncio.component.run(
        [component],
        True,
        'trace' if os.environ.get('WAMP_DEBUG') == 'true' else 'critical'
    )
