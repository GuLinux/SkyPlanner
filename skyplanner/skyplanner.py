from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash, json
from skyplanner.models.db import db
from flask_login import LoginManager

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


@app.route('/api/login', methods=['POST'])
def login():
    data = request.get_json()
    user = User.query.filter_by(username=data['username']).first()
    if not user or not user.verify_password(data['password']):
        return json.jsonify({'result': 'wrong_user_or_password'})
    return json.jsonify({'result': 'ok'})

