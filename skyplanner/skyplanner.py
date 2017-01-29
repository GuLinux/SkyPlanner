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
from skyplanner.route_helpers import json_ok, json_error, users_controller, auth_url, skyplanner_api
from skyplanner.models.skyobject import SkyObject
from skyplanner.models.telescope import Telescope
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
    except Exception as e:
        app.logger.debug("Login error.", exc_info = True)
        return json_error("Bad request"), 400

@app.route('/api/users/create', methods=['PUT'])
def create_user():
    try:
        return json.jsonify(users_controller().create(request.get_json())), 201
    except UsersController.Error as e:
        return json_error(reason=e.reason), 409

@skyplanner_api('/api/users/get', auth_required=True)
def get_user(user):
    return json.jsonify(user.to_map())

@skyplanner_api('/api/telescopes', auth_required = True)
def get_telescopes(user):
    return json.jsonify([t.to_map() for t in user.telescopes])

@app.route('/api/telescopes', methods=['PUT'])
@auth_url
def add_telescope(user):
    try:
        data = request.get_json()
        telescope = Telescope(data['name'], data['focal_length'], data['diameter'], user)
        if not telescope.validate()[0]:
            return json_error("Bad request", errors=telescope.validate()[1]), 400
        db.session.add(telescope)
        db.session.commit()
        return json.jsonify(telescope.to_map()), 201
    except Exception as e:
        app.logger.debug("Create telescope error.", exc_info = True)
        return json_error("Bad request"), 400

@app.route('/api/telescopes/<id>', methods=['POST'])
@auth_url
def edit_telescope(user, id):
    telescope = Telescope.query.filter_by(id = id).one()
    telescope.update(request.get_json())
    if not telescope.validate()[0]:
        return json_error("Bad request", errors=telescope.validate()[1]), 400
    db.session.commit()
    return json.jsonify(telescope.to_map()), 200
    

@app.route('/api/telescopes/<id>', methods=['DELETE'])
@auth_url
def delete_telescope(user, id):
    try:
        result = Telescope.query.filter_by(id = id).delete()
        app.logger.debug(result)
        db.session.commit()
        if result == 1:
            return json_ok(), 200
        else:
            return json_error("not found"), 404
        
    except Exception as e:
        app.logger.debug("Delete telescope error.", exc_info = True)
        return json_error("Bad request"), 400

@app.cli.command()
def init_db():
    """Initialize the database"""
    db.create_all()

@skyplanner_api(url='/api_test', auth_required=True)
def api_test(user):
    return '[GET] hello, user: {0}\n'.format(user.to_map()), 200

@skyplanner_api(url='/api_test2', methods=['POST'])
def api_test2():
    return '[POST] hello\n', 200

