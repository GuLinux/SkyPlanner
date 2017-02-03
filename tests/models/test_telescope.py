import unittest
from skyplanner.models.telescope import Telescope

# limiting magnitude = 5 x LOG10(aperture of scope in cm)  + 7.5
# math.log10(x)

class FakeUser:
    def __init__(self):
        self.id = 0

class TestTelescope(unittest.TestCase):
    def test_validation(self):
        telescope = Telescope('', '', '', FakeUser()) 
        validate = telescope.validate()
        self.assertFalse(validate[0])
        self.assertEqual(['name is not valid', 'focal_length is not valid', 'diameter is not valid'], validate[1])
