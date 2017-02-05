import unittest
from skyplanner.models.telescope import Telescope

# limiting magnitude = 5 x LOG10(aperture of scope in cm)  + 7.5
# math.log10(x)

class FakeUser:
    def __init__(self):
        self.id = 0

class TestTelescope(unittest.TestCase):
    def test_invalid(self):
        telescope = Telescope('', '', '', FakeUser()) 
        validate = telescope.validate()
        self.assertFalse(validate[0])
        self.assertEqual(['name is not valid', 'focal_length is not valid', 'diameter is not valid'], validate[1])
        self.assertFalse(Telescope('a', 'b', 100, FakeUser()).validate()[0])
        self.assertFalse(Telescope('a', 0, 100, FakeUser()).validate()[0])
        self.assertFalse(Telescope('a', 200, 'c', FakeUser()).validate()[0])
        self.assertFalse(Telescope('a', 200, 0, FakeUser()).validate()[0])
        self.assertFalse(Telescope('', 200, 20, FakeUser()).validate()[0])

    def test_valid(self):
        telescope = Telescope('name', 1200, 200, FakeUser())
        validate = telescope.validate()
        self.assertTrue(validate[0])
        self.assertEqual(0, len(validate[1]))

    def test_magnitude(self):
        telescope = Telescope('test', 100, 120, FakeUser())
        self.assertAlmostEqual(10.395906230, telescope.magnitude_gain() )
