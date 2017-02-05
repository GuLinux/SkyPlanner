from skyplanner.models.db import db
import math
import pprint
import sys

class Telescope(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String)
    focal_length = db.Column(db.Integer)
    diameter = db.Column(db.Integer)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'))

    def __init__(self, name, focal_length, diameter, user):
        self.name = name
        self.focal_length = focal_length
        self.diameter = diameter
        self.user_id = user.id

    def magnitude_gain(self, pupil = None):
        if not pupil:
            pupil = self.user.pupil
        return math.log10(self.diameter / pupil) * 5

    def validate(self):
        is_number = lambda n: isinstance(n, int) or isinstance(n, float)
        errors = []
        if not self.name:
            errors.append('name is not valid')
        if not self.focal_length or not is_number(self.focal_length): 
            errors.append('focal_length is not valid')
        if not self.diameter or not is_number(self.diameter):
            errors.append('diameter is not valid')
        return len(errors) == 0, errors

    def update(self, data):
        if 'name' in data:
            self.name = data['name']
        if 'focal_length' in data:
            self.focal_length = data['focal_length']
        if 'diameter' in data:
            self.diameter = data['diameter']

    def to_map(self):
        return {'id': self.id, 'name': self.name, 'focal_length': self.focal_length, 'diameter': self.diameter }

    def __str__(self):
        return 'id: {0}, name: {1}, focal_length: {2}, diameter: {3}, user_id: {4}' \
                .format(self.id, self.name, self.focal_length, self.diameter, self.user_id)

    def __repr__(self):
        return self.__str__()

