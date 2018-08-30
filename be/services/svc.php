<?php
session_start();
use \Psr\Http\Message\ServerRequestInterface as Request;
use \Psr\Http\Message\ResponseInterface as Response;

require '../config.php';
require '../vendor/autoload.php';
require 'UsersCtrl.php';
require 'AuthCtrl.php';

$app = new \Slim\App(['settings' => $config]);
$container = $app->getContainer();
$container['db'] = function ($container) {
    $db = $container["settings"]["db"];
    $pdo = new PDO('mysql:host=' . $db['host'] . ';dbname=' . $db['dbname'], $db['user'], $db['pass']);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $pdo->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
    $pdo->setAttribute(PDO::ATTR_STRINGIFY_FETCHES, false);
    $pdo->setAttribute(PDO::ATTR_EMULATE_PREPARES, false);
    return $pdo;
};

$app->get('/auth/isLogged', function (Request $request, Response $response, array $args) {
    $isLogged = AuthCtrl::isLogged();
    return $response->withJson(['isLogged' => $isLogged]);
});

$app->post('/auth/login', function (Request $request, Response $response, array $args) {
    $authCtrl = new AuthCtrl();
    $json = $request->getBody();
    $payload = json_decode($json, true);
    $logged = $authCtrl->login($payload['password']);
    return $response->withJson(['logged' => $logged]);
});

$app->get('/users', function (Request $request, Response $response, array $args) {
    AuthCtrl::assertIsLogged();
    // sleep(1);
    $usersCtrl = new UsersCtrl($this->db);
    $respArray = $usersCtrl->getUsers();
    return $response->withJson($respArray);
});

$app->post('/users/update_ratings', function (Request $request, Response $response, array $args) {
    AuthCtrl::assertIsLogged();
    // sleep(1);

    $json = $request->getBody();
    $usersCodes = json_decode($json, true);
    $winnerUserNid = $usersCodes['winnerUserNid'];
    $looserUserNid = $usersCodes['looserUserNid'];

    $usersCtrl = new UsersCtrl($this->db);
    $usersCtrl->updateRatings($winnerUserNid, $looserUserNid);
    return $response->withJson([]);
});

$app->run();
