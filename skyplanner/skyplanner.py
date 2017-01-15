from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash

app = Flask(__name__)

app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)

@app.route('/')
def index():
    return render_template('index.html')



