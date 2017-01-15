from flask import Flask, request, session, g, redirect, url_for, abort, \
     render_template, flash
import glob

app = Flask(__name__)

app.config.from_envvar('SKYPLANNER_SETTINGS', silent=True)
app.config['js_app_files'] = [x.replace(app.static_folder, app.static_url_path) for x in glob.glob(app.static_folder + '/app/*.js')]
@app.route('/')
def index():
    return render_template('index.html', js_app_files = app.config['js_app_files'])



