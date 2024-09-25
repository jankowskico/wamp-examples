#!/usr/bin/node
const DEFAULT_ENV = '.env.service';

if (require('fs').existsSync(DEFAULT_ENV)) {
    require('dotenv').config({ path: DEFAULT_ENV })
}

if (
    !process.env.WAMP_DEBUG
    || !process.env.WAMP_HOST
    || !process.env.WAMP_PORT
    || !process.env.WAMP_AUTHID
    || !process.env.WAMP_SECRET
    || !process.env.WAMP_REALM
) {
    process.exit();
}

AUTOBAHN_DEBUG = (process.env.WAMP_DEBUG == "true");

var autobahn = require('autobahn');
var connection = new autobahn.Connection({
    url: 'wss://'+ process.env.WAMP_HOST +':'+ process.env.WAMP_PORT,
    realm: process.env.WAMP_REALM,
    authmethods: ['wampcra'],
    authid: process.env.WAMP_AUTHID,
    onchallenge: function (session, method, extra) {
        if (method === 'wampcra') {
            return autobahn.auth_cra.sign(
                process.env.WAMP_SECRET,
                extra.challenge
            );
        }
    }
});

connection.onopen = function(session) {

    setInterval(function() {
        let message = [
            'This is js first argument.',
            'This is js second argument.'
        ];
        session.publish('test', message);
        console.log('Sent:', message);
    }, 1000);

};

connection.open();
