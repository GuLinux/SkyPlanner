from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash, json
import click
from skyplanner.models.db import db
from skyplanner.controllers.users import UsersController
import pprint

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/test.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)
app.config['SECRET_KEY'] = 'a51df902b5a78f3ced5074db638e9992cd8ffac1547514449d9426f67eb90ed6'
db.init_app(app)

import skyplanner.route_helpers
skyplanner.route_helpers.app = app
from skyplanner.route_helpers import json_ok, json_error, users_controller, auth_url
from skyplanner.models.skyobject import SkyObject
from skyplanner.models.user import User

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/users/login', methods=['POST'])
def login():
    try:
        return json.jsonify(users_controller().login(request.get_json()))
    except UsersController.Error as e:
        return json_error(reason=e.reason), 401 

@app.route('/api/users/create', methods=['PUT'])
def create_user():
    try:
        return json.jsonify(users_controller().create(request.get_json()))
    except UsersController.Error as e:
        return json_error(reason=e.reason), 409

@app.route('/api/users/get')
@auth_url
def get_user(user):
    return json.jsonify(user.to_map())


@app.cli.command()
def init_db():
    """Initialize the database"""
    db.create_all()

