import unittest
from skyplanner.ephemeris import SkySector
from skyplanner.models import Angle, SkyCoord
from skyplanner.ephemeris.native.ephemeris import *

import time

class TestSkySector(unittest.TestCase):
    def setUp(self):
        self.sectors = SkySector.all()
        
    def test_sectors_size(self):
        self.assertEqual( 360/SkySector.DEGREES_STEP * 180/SkySector.DEGREES_STEP, len(self.sectors))


    def test_sector_properties(self):
        self.assertEqual(Angle(0), self.sectors[0].start.ra)
        self.assertEqual(Angle(-90), self.sectors[0].start.dec)

        self.assertEqual(Angle(SkySector.DEGREES_STEP), self.sectors[0].end.ra)
        self.assertEqual(Angle(SkySector.DEGREES_STEP -90), self.sectors[0].end.dec)

        self.assertEqual(Angle(SkySector.DEGREES_STEP/2), self.sectors[0].center.ra)
        self.assertEqual(Angle(SkySector.DEGREES_STEP/2 -90), self.sectors[0].center.dec)

    def test_find_sector(self):
        ra = Angle(hours=12.53)
        dec = Angle(33.24)
        sector = SkySector.find_sector( SkyCoord(ra, dec) )
        self.assertTrue( sector.start.ra < ra < sector.end.ra) 
        self.assertTrue( sector.start.dec < dec < sector.end.dec) 

    def test_benchmark_ephemeris(self):
        avg = 0
        rst = []
        cnt = []
        size = 50
        for n in range(0, size):
            ephemeris = Ephemeris(51.507351, 0.127758)
            start = time.time()
            rts = SkySector.rise_transit_set(ephemeris, 2457767.500000)
            end = time.time()
            cnt = [0, 0, 0]
            for obj in rts:
                if obj.type is RiseTransitSet.Type.CircumPolar:
                    cnt[0] += 1
                elif obj.type is RiseTransitSet.Type.Rises:
                    cnt[1] += 1
                elif obj.type is RiseTransitSet.Type.NeverRises:
                    cnt[2] += 1
            avg += end-start
        print('elapsed: {0} ms; total objects: {1}, circumpolar: {2}, rising: {3}, never rising: {4})'.format( (avg/size)*1000, len(rts), cnt[0], cnt[1], cnt[2] ))

