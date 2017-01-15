import glob
from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash
from skyplanner.models.db import db
from flask_login import LoginManager

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////tmp/test.db'
app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)
app.config['js_app_files'] = [x.replace(app.static_folder, app.static_url_path) for x in glob.glob(app.static_folder + '/app/*.js')]

db.init_app(app)
login_manager = LoginManager()
login_manager.init_app(app)

from skyplanner.models.skyobject import SkyObject

@app.route('/')
def index():
    return render_template('index.html', js_app_files = app.config['js_app_files'])



