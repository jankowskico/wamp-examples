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

$client = new \Thruway\Peer\Client($_ENV['WAMP_REALM']);
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

$client->on('open', function (\Thruway\ClientSession $session) {

    $session->register('test', function ($request) {
        // $response = [
        //     'This is php response first argument.',
        //     'This is php response second argument.'
        // ];
        $response = 'This is php response.';
        echo 'Received: '.json_encode($request)."\n";
        echo 'Sent: '.json_encode($response)."\n";
        return $response;
    });

});

$client->start();
