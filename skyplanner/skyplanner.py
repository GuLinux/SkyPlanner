from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash, json
import click
from .models import db

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/test.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)
app.config['SECRET_KEY'] = 'a51df902b5a78f3ced5074db638e9992cd8ffac1547514449d9426f67eb90ed6'
db.init_app(app)

import skyplanner.route_helpers
skyplanner.route_helpers.app = app
from .route_helpers import json_ok, json_error, users_controller, skyplanner_api, telescopes_controller
from .models import SkyObject, Telescope, Observation, User
from .errors import SkyPlannerError

@app.route('/')
def index():
    return render_template('index.html')

@skyplanner_api(url='/api/users/login', methods=['POST'])
def login():
    return json.jsonify(users_controller().login(request.get_json()))

@skyplanner_api('/api/users/create', methods=['PUT'])
def create_user():
    return json.jsonify(users_controller().create(request.get_json())), 201

@skyplanner_api(url='/api/users/get', auth_required=True)
def get_user(user):
    return json.jsonify(user.to_map())

@skyplanner_api(url='/api/telescopes', auth_required = True)
def get_telescopes(user):
    return json.jsonify(telescopes_controller().all(user))

@skyplanner_api(url='/api/telescopes', methods=['PUT'], auth_required=True)
def add_telescope(user):
    return json.jsonify(telescopes_controller().add(user, request.get_json())), 201

@skyplanner_api(url='/api/telescopes/<id>', methods=['POST'], auth_required=True)
def edit_telescope(user, id):
    return json.jsonify(telescopes_controller().edit(user, id, request.get_json())), 200

@skyplanner_api(url='/api/telescopes/<id>', methods=['DELETE'], auth_required = True)
def delete_telescope(user, id):
    return json.jsonify(telescopes_controller().remove(user, id)), 200

@skyplanner_api(url='/api/observations', auth_required = True)
def get_observations(user):
    return json.jsonify(observations_controller().all(user))

@skyplanner_api(url='/api/observations', auth_required = True, methods = ['PUT'])
def create_observation(user):
    return json.jsonify(observations_controller().create(user, request.get_json()))

@skyplanner_api(url='/api/observations<id>', auth_required = True, methods = ['POST'])
def edit_observation(user, id):
    return json.jsonify(observations_controller().update(user, id, request.get_json()))

@skyplanner_api(url='/api/observations<id>', auth_required = True, methods = ['DELETE'])
def delete_observation(user, id):
    return json.jsonify(observations_controller().remove(user, id))



#########  CLI Commands
@app.cli.command()
def init_db():
    """Initialize the database"""
    db.create_all()

