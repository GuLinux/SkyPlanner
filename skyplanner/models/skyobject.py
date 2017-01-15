from skyplanner.models.db import db

class SkyObject(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    ra = db.Column(db.Float(precision=10), nullable = False)
    dec = db.Column(db.Float(precision=10), nullable = False)
