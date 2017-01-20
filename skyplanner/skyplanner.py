from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash, json
import click
from skyplanner.models.db import db
import pprint
from skyplanner.controllers.users import UsersController

controllers = dict()

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/test.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)
app.config['SECRET_KEY'] = 'a51df902b5a78f3ced5074db638e9992cd8ffac1547514449d9426f67eb90ed6'

db.init_app(app)

from skyplanner.models.skyobject import SkyObject
from skyplanner.models.user import User

def auth_url(func):
    def wrapper(*args, **kwargs):
        user = None
        if request.args.get('auth'):
            user = usersController().verify_token(request.args.get('auth'))
        if not user:
            return json.jsonify({'result': 'error', 'reason': 'auth_required'}), 401
        return func(*args, **kwargs, user=user)
    return wrapper

@app.route('/')
def index():
    return render_template('index.html')


@app.route('/api/users/login', methods=['POST'])
def login():
    return json.jsonify(usersController().login(request.get_json()))

@app.route('/api/users/create', methods=['PUT'])
def create_user():
    return json.jsonify(usersController().create(request.get_json()))

@app.route('/api/users/get')
@auth_url
def get_user(user):
    return json.jsonify(user.to_map())

def usersController():
    def create():
        return UsersController(app)
    return get_controller('users', create)

def get_controller(name, factory):
    if not name in controllers:
        controllers[name] = factory()
    return controllers[name]

@app.cli.command()
def init_db():
    """Initialize the database"""
    db.create_all()

