import math

class Angle:
    __rad = 180 / math.pi
    __hour = 360 / 24

    def __init__(self, degrees = None, radians = None, hours = None):
        self.degrees = None
        self.set(degrees = degrees, radians = radians, hours = hours)

    def set(self, degrees = None, radians = None, hours = None):
        if not degrees is None:
            self.degrees = degrees
        elif not radians is None:
            self.degrees = radians * Angle.__rad
        elif not hours is None:
            self.degrees = hours * Angle.__hour

    def radians(self):
        return self.degrees / Angle.__rad

    def hours(self):
        return self.degrees / Angle.__hour

    def __str__(self):
        return '{0}deg'.format(self.degrees)

    def __repr__(self):
        return self.__str__()

    def __add__(self, other):
        return Angle(degrees = self.degrees + self.__get_deg(other))

    def __mul__(self, other):
        return Angle(degrees = self.degrees * self.__get_deg(other))

    def __sub__(self, other):
        return Angle(degrees = self.degrees - self.__get_deg(other))

    def __truediv__(self, other):
        return Angle(degrees = self.degrees / self.__get_deg(other))

    def __iadd__(self, other):
        self.degrees += self.__get_deg(other)
        return self

    def __isub__(self, other):
        self.degrees -= self.__get_deg(other)
        return self

    def __imul__(self, other):
        self.degrees *= self.__get_deg(other)
        return self

    def __itruediv__(self, other):
        self.degrees /= self.__get_deg(other)
        return self

    def __hash__(self):
        return hash(self.degrees)

    def __eq__(self, other):
        return self.degrees == other.degrees

    def __lt__(self, other):
        return self.degrees < self.__get_deg(other)

    def __le__(self, other):
        return self.degrees <= self.__get_deg(other)

    def __gt__(self, other):
        return self.degrees > self.__get_deg(other)

    def __ge__(self, other):
        return self.degrees >= self.__get_deg(other)

    def __get_deg(self, other):
        return other.degrees if type(other) is Angle else other
