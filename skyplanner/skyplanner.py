from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash, json
import click
from skyplanner.models.db import db
from flask_login import LoginManager
import pprint
from skyplanner.controllers.users import UsersController

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/test.db'
app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)

db.init_app(app)
login_manager = LoginManager()
login_manager.init_app(app)

from skyplanner.models.skyobject import SkyObject
from skyplanner.models.user import User

@app.route('/')
def index():
    return render_template('index.html')


@app.route('/api/users/login', methods=['POST'])
def login():
    return json.jsonify(usersController().login(request.get_json()))

@app.route('/api/users/create', methods=['PUT'])
def create_user():
    return json.jsonify(usersController().create(request.get_json()))

def usersController():
    return UsersController(app)

@app.cli.command()
def init_db():
    """Initialize the database"""
    db.create_all()

