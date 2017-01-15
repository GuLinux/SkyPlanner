from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash
from skyplanner.models.db import db
from flask_login import LoginManager

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/test.db'
app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)

db.init_app(app)
login_manager = LoginManager()
login_manager.init_app(app)

from skyplanner.models.skyobject import SkyObject

@app.route('/')
def index():
    return render_template('index.html')


