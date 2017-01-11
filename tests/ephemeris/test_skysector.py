import unittest
from ephem import degrees, degree
from skyplanner.ephemeris.skysector import SkySector

class TestSkySector(unittest.TestCase):
    def setUp(self):
        self.sectors = SkySector.all()
        
    def test_sectors_size(self):
        self.assertEqual( 360/SkySector.DEGREES_STEP * 180/SkySector.DEGREES_STEP, len(self.sectors))


    def test_sector_properties(self):
        step_angle = degrees(SkySector.DEGREES_STEP * degree)
        self.assertEqual(0, self.sectors[0].ar[0])
        self.assertEqual(degrees(degree * -90), self.sectors[0].dec[0])

        self.assertEqual(step_angle, self.sectors[0].ar[1])
        self.assertEqual(step_angle + degrees(degree*-90), self.sectors[0].dec[1])

        self.assertEqual(step_angle/2, self.sectors[0].center['ar'])
        self.assertEqual(step_angle/2 + degrees(degree*-90), self.sectors[0].center['dec'])
