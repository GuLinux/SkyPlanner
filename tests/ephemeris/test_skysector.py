import unittest
from ephem import degrees, degree, hours, Observer
from skyplanner.ephemeris.skysector import SkySector
from math import pi
import time

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

    def test_find_sector(self):
        hour = pi/12
        ar = hours(hour*12.53)
        dec = degrees(degree*33.24)
        sector = SkySector.find_sector( ar, dec )
        self.assertTrue( sector.ar[0] < ar < sector.ar[1]) 
        self.assertTrue( sector.dec[0] < dec < sector.dec[1]) 

    def test_benchmark_ephemeris(self):
        observer = Observer()
        observer.lat = degrees(degree*51.507351)
        observer.lon = degrees(degree*-0.127758)
        start = time.time()
        rts = SkySector.rise_transit_set(observer)
        end = time.time()
        print('elapsed: {0} seconds; ephemeris size: {1} ({2} total objects)'.format(end-start, len(rts), len(self.sectors)))

