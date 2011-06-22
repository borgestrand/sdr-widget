/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include "Frequency.h"

Frequency::Frequency()
{
    info.clear();
    info << FrequencyInfo(60000LL, 60000LL, "MSF Time Signal",                     BAND_GEN,FALSE);
    info << FrequencyInfo(75000LL, 75000LL, "HGB Time Signal",                   BAND_GEN,FALSE);
    info << FrequencyInfo(77500LL, 77500LL, "DCF77 Time Signal",                   BAND_GEN,FALSE);
    info << FrequencyInfo(153000LL, 279000LL, "AM - Long Wave",                    BAND_GEN,FALSE);
    info << FrequencyInfo(530000LL, 1710000LL, "Broadcast AM Med Wave",            BAND_GEN,FALSE);
    info << FrequencyInfo(1800000LL, 1809999LL, "160M CW/Digital Modes",            BAND_160, TRUE);
    info << FrequencyInfo(1810000LL, 1810000LL, "160M CW QRP",                      BAND_160, TRUE);
    info << FrequencyInfo(1810001LL, 1842999LL, "160M CW",                          BAND_160, TRUE);
    info << FrequencyInfo(1843000LL, 1909999LL, "160M SSB/SSTV/Wide band",          BAND_160, TRUE);
    info << FrequencyInfo(1910000LL, 1910000LL, "160M SSB QRP",                     BAND_160, TRUE);
    info << FrequencyInfo(1910001LL, 1994999LL, "160M SSB/SSTV/Wide band",          BAND_160, TRUE);
    info << FrequencyInfo(1995000LL, 1999999LL, "160M Experimental",                BAND_160, TRUE);

    info << FrequencyInfo(2300000LL, 2495000LL, "120M Short Wave",                  BAND_GEN,FALSE);

    info << FrequencyInfo(2500000LL, 2500000LL, "WWV",                              BAND_WWV,FALSE);

    info << FrequencyInfo(3200000LL, 3400000LL, "90M Short Wave",                   BAND_GEN,FALSE);

    info << FrequencyInfo(3500000LL, 3524999LL, "80M Extra CW",                     BAND_80, TRUE);
    info << FrequencyInfo(3525000LL, 3579999LL, "80M CW",                           BAND_80, TRUE);
    info << FrequencyInfo(3580000LL, 3589999LL, "80M RTTY",                         BAND_80, TRUE);
    info << FrequencyInfo(3590000LL, 3590000LL, "80M RTTY DX",                      BAND_80, TRUE);
    info << FrequencyInfo(3590001LL, 3599999LL, "80M RTTY",                         BAND_80, TRUE);
    info << FrequencyInfo(3600000LL, 3699999LL, "75M Extra SSB",                    BAND_80, TRUE);
    info << FrequencyInfo(3700000LL, 3789999LL, "75M Ext/Adv SSB",                  BAND_80, TRUE);
    info << FrequencyInfo(3790000LL, 3799999LL, "75M Ext/Adv DX Window",            BAND_80, TRUE);
    info << FrequencyInfo(3800000LL, 3844999LL, "75M SSB",                          BAND_80, TRUE);
    info << FrequencyInfo(3845000LL, 3845000LL, "75M SSTV",                         BAND_80, TRUE);
    info << FrequencyInfo(3845001LL, 3884999LL, "75M SSB",                          BAND_80, TRUE);
    info << FrequencyInfo(3885000LL, 3885000LL, "75M AM Calling Frequency",         BAND_80, TRUE);
    info << FrequencyInfo(3885001LL, 3999999LL, "75M SSB",                          BAND_80, TRUE);
    info << FrequencyInfo(4750000LL, 4999999LL, "60M Short Wave",                   BAND_GEN,FALSE);

    info << FrequencyInfo(5000000LL, 5000000LL, "WWV",                              BAND_WWV,FALSE);

    info << FrequencyInfo(5330500LL, 5330500LL, "60M Channel 1",                    BAND_60, TRUE);
    info << FrequencyInfo(5346500LL, 5346500LL, "60M Channel 2",                    BAND_60, TRUE);
    info << FrequencyInfo(5366500LL, 5366500LL, "60M Channel 3",                    BAND_60, TRUE);
    info << FrequencyInfo(5371500LL, 5371500LL, "60M Channel 4",                    BAND_60, TRUE);
    info << FrequencyInfo(5403500LL, 5403500LL, "60M Channel 5",                    BAND_60, TRUE);

    info << FrequencyInfo(5900000LL, 6200000LL, "49M Short Wave",                   BAND_GEN,FALSE);

    info << FrequencyInfo(7000000LL, 7024999LL, "40M Extra CW",                     BAND_40, TRUE);
    info << FrequencyInfo(7025000LL, 7039999LL, "40M CW",                           BAND_40, TRUE);
    info << FrequencyInfo(7040000LL, 7040000LL, "40M RTTY DX",                      BAND_40, TRUE);
    info << FrequencyInfo(7040001LL, 7099999LL, "40M RTTY",                         BAND_40, TRUE);
    info << FrequencyInfo(7100000LL, 7124999LL, "40M CW",                           BAND_40, TRUE);
    info << FrequencyInfo(7125000LL, 7170999LL, "40M Ext/Adv SSB",                  BAND_40, TRUE);
    info << FrequencyInfo(7171000LL, 7171000LL, "40M SSTV",                         BAND_40, TRUE);
    info << FrequencyInfo(7171001LL, 7174999LL, "40M Ext/Adv SSB",                  BAND_40, TRUE);
    info << FrequencyInfo(7175000LL, 7289999LL, "40M SSB",                          BAND_40, TRUE);
    info << FrequencyInfo(7290000LL, 7290000LL, "40M AM Calling Frequency",         BAND_40, TRUE);
    info << FrequencyInfo(7290001LL, 7299999LL, "40M SSB",                          BAND_40, TRUE);

    info << FrequencyInfo(7300000LL, 7350000LL, "41M Short Wave",                   BAND_GEN,FALSE);
    info << FrequencyInfo(9400000LL, 9900000LL, "31M Short Wave",                   BAND_GEN,FALSE);

    info << FrequencyInfo(10000000LL, 10000000LL, "WWV",                            BAND_WWV,FALSE);

    info << FrequencyInfo(10100000LL, 10129999LL, "30M CW",                         BAND_30, TRUE);
    info << FrequencyInfo(10130000LL, 10139999LL, "30M RTTY",                       BAND_30, TRUE);
    info << FrequencyInfo(10140000LL, 10149999LL, "30M Packet",                     BAND_30, TRUE);

    info << FrequencyInfo(11600000LL, 12100000LL, "25M Short Wave",                 BAND_GEN,FALSE);
    info << FrequencyInfo(13570000LL, 13870000LL, "22M Short Wave",                 BAND_GEN,FALSE);

    info << FrequencyInfo(14000000LL, 14024999LL, "20M Extra CW",                   BAND_20, TRUE);
    info << FrequencyInfo(14025000LL, 14069999LL, "20M CW",                         BAND_20, TRUE);
    info << FrequencyInfo(14070000LL, 14094999LL, "20M RTTY",                       BAND_20, TRUE);
    info << FrequencyInfo(14095000LL, 14099499LL, "20M Packet",                     BAND_20, TRUE);
    info << FrequencyInfo(14099500LL, 14099999LL, "20M CW",                         BAND_20, TRUE);
    info << FrequencyInfo(14100000LL, 14100000LL, "20M NCDXF Beacons",              BAND_20, TRUE);
    info << FrequencyInfo(14100001LL, 14100499LL, "20M CW",                         BAND_20, TRUE);
    info << FrequencyInfo(14100500LL, 14111999LL, "20M Packet",                     BAND_20, TRUE);
    info << FrequencyInfo(14112000LL, 14149999LL, "20M CW",                         BAND_20, TRUE);
    info << FrequencyInfo(14150000LL, 14174999LL, "20M Extra SSB",                  BAND_20, TRUE);
    info << FrequencyInfo(14175000LL, 14224999LL, "20M Ext/Adv SSB",                BAND_20, TRUE);
    info << FrequencyInfo(14225000LL, 14229999LL, "20M SSB",                        BAND_20, TRUE);
    info << FrequencyInfo(14230000LL, 14230000LL, "20M SSTV",                       BAND_20, TRUE);
    info << FrequencyInfo(14230000LL, 14285999LL, "20M SSB",                        BAND_20, TRUE);
    info << FrequencyInfo(14286000LL, 14286000LL, "20M AM Calling Frequency",       BAND_20, TRUE);
    info << FrequencyInfo(14286001LL, 14349999LL, "20M SSB",                        BAND_20, TRUE);

    info << FrequencyInfo(15000000LL, 15000000LL, "WWV",                            BAND_WWV,FALSE);

    info << FrequencyInfo(15100000LL, 15800000LL, "19M Short Wave",                 BAND_GEN,FALSE);
    info << FrequencyInfo(17480000LL, 17900000LL, "16M Short Wave",                 BAND_GEN,FALSE);

    info << FrequencyInfo(18068000LL, 18099999LL, "17M CW",                         BAND_17, TRUE);
    info << FrequencyInfo(18100000LL, 18104999LL, "17M RTTY",                       BAND_17, TRUE);
    info << FrequencyInfo(18105000LL, 18109999LL, "17M Packet",                     BAND_17, TRUE);
    info << FrequencyInfo(18110000LL, 18110000LL, "17M NCDXF Beacons",              BAND_17, TRUE);
    info << FrequencyInfo(18110001LL, 18167999LL, "17M SSB",                        BAND_17, TRUE);

    info << FrequencyInfo(18900000LL, 19020000LL, "15M Short Wave",                 BAND_GEN,FALSE);

    info << FrequencyInfo(20000000LL, 20000000LL, "WWV",                            BAND_WWV,FALSE);

    info << FrequencyInfo(21000000LL, 21024999LL, "15M Extra CW",                   BAND_15, TRUE);
    info << FrequencyInfo(21025000LL, 21069999LL, "15M CW",                         BAND_15, TRUE);
    info << FrequencyInfo(21070000LL, 21099999LL, "15M RTTY",                       BAND_15, TRUE);
    info << FrequencyInfo(21100000LL, 21109999LL, "15M Packet",                     BAND_15, TRUE);
    info << FrequencyInfo(21110000LL, 21149999LL, "15M CW",                         BAND_15, TRUE);
    info << FrequencyInfo(21150000LL, 21150000LL, "15M NCDXF Beacons",              BAND_15, TRUE);
    info << FrequencyInfo(21150001LL, 21199999LL, "15M CW",                         BAND_15, TRUE);
    info << FrequencyInfo(21200000LL, 21224999LL, "15M Extra SSB",                  BAND_15, TRUE);
    info << FrequencyInfo(21225000LL, 21274999LL, "15M Ext/Adv SSB",                BAND_15, TRUE);
    info << FrequencyInfo(21275000LL, 21339999LL, "15M SSB",                        BAND_15, TRUE);
    info << FrequencyInfo(21340000LL, 21340000LL, "15M SSTV",                       BAND_15, TRUE);
    info << FrequencyInfo(21340001LL, 21449999LL, "15M SSB",                        BAND_15, TRUE);

    info << FrequencyInfo(21450000LL, 21850000LL, "13M Short Wave",                 BAND_GEN,FALSE);

    info << FrequencyInfo(24890000LL, 24919999LL, "12M CW",                         BAND_12, TRUE);
    info << FrequencyInfo(24920000LL, 24924999LL, "12M RTTY",                       BAND_12, TRUE);
    info << FrequencyInfo(24925000LL, 24929999LL, "12M Packet",                     BAND_12, TRUE);
    info << FrequencyInfo(24930000LL, 24930000LL, "12M NCDXF Beacons",              BAND_12, TRUE);

    info << FrequencyInfo(25600000LL, 26100000LL, "11M Short Wave",                 BAND_GEN,FALSE);

    info << FrequencyInfo(28000000LL, 28069999LL, "10M CW",                         BAND_10, TRUE);
    info << FrequencyInfo(28070000LL, 28149999LL, "10M RTTY",                       BAND_10, TRUE);
    info << FrequencyInfo(28150000LL, 28199999LL, "10M CW",                         BAND_10, TRUE);
    info << FrequencyInfo(28200000LL, 28200000LL, "10M NCDXF Beacons",              BAND_10, TRUE);
    info << FrequencyInfo(28200001LL, 28299999LL, "10M Beacons",                    BAND_10, TRUE);
    info << FrequencyInfo(28300000LL, 28679999LL, "10M SSB",                        BAND_10, TRUE);
    info << FrequencyInfo(28680000LL, 28680000LL, "10M SSTV",                       BAND_10, TRUE);
    info << FrequencyInfo(28680001LL, 28999999LL, "10M SSB",                        BAND_10, TRUE);
    info << FrequencyInfo(29000000LL, 29199999LL, "10M AM",                         BAND_10, TRUE);
    info << FrequencyInfo(29200000LL, 29299999LL, "10M SSB",                        BAND_10, TRUE);
    info << FrequencyInfo(29300000LL, 29509999LL, "10M Satellite Downlinks",        BAND_10, TRUE);
    info << FrequencyInfo(29510000LL, 29519999LL, "10M DeadBAND_",                   BAND_10, TRUE);
    info << FrequencyInfo(29520000LL, 29589999LL, "10M Repeater Inputs",            BAND_10, TRUE);
    info << FrequencyInfo(29590000LL, 29599999LL, "10M DeadBAND_",                   BAND_10, TRUE);
    info << FrequencyInfo(29600000LL, 29600000LL, "10M FM Simplex",                 BAND_10, TRUE);
    info << FrequencyInfo(29600001LL, 29609999LL, "10M DeadBAND_",                   BAND_10, TRUE);
    info << FrequencyInfo(29610000LL, 29699999LL, "10M Repeater Outputs",           BAND_10, TRUE);

    info << FrequencyInfo(50000000LL, 50059999LL, "6M CW",                          BAND_6, TRUE);
    info << FrequencyInfo(50060000LL, 50079999LL, "6M Beacon Sub-BAND_",             BAND_6, TRUE);
    info << FrequencyInfo(50080000LL, 50099999LL, "6M CW",                          BAND_6, TRUE);
    info << FrequencyInfo(50100000LL, 50124999LL, "6M DX Window",                   BAND_6, TRUE);
    info << FrequencyInfo(50125000LL, 50125000LL, "6M Calling Frequency",           BAND_6, TRUE);
    info << FrequencyInfo(50125001LL, 50299999LL, "6M SSB",                         BAND_6, TRUE);
    info << FrequencyInfo(50300000LL, 50599999LL, "6M All Modes",                   BAND_6, TRUE);
    info << FrequencyInfo(50600000LL, 50619999LL, "6M Non Voice",                   BAND_6, TRUE);
    info << FrequencyInfo(50620000LL, 50620000LL, "6M Digital Packet Calling",      BAND_6, TRUE);
    info << FrequencyInfo(50620001LL, 50799999LL, "6M Non Voice",                   BAND_6, TRUE);
    info << FrequencyInfo(50800000LL, 50999999LL, "6M RC",                          BAND_6, TRUE);
    info << FrequencyInfo(51000000LL, 51099999LL, "6M Pacific DX Window",           BAND_6, TRUE);
    info << FrequencyInfo(51100000LL, 51119999LL, "6M DeadBAND_",                    BAND_6, TRUE);
    info << FrequencyInfo(51120000LL, 51179999LL, "6M Digital Repeater Inputs",     BAND_6, TRUE);
    info << FrequencyInfo(51180000LL, 51479999LL, "6M Repeater Inputs",             BAND_6, TRUE);
    info << FrequencyInfo(51480000LL, 51619999LL, "6M DeadBAND_",                    BAND_6, TRUE);
    info << FrequencyInfo(51620000LL, 51679999LL, "6M Digital Repeater Outputs",    BAND_6, TRUE);
    info << FrequencyInfo(51680000LL, 51979999LL, "6M Repeater Outputs",            BAND_6, TRUE);
    info << FrequencyInfo(51980000LL, 51999999LL, "6M DeadBAND_",                    BAND_6, TRUE);
    info << FrequencyInfo(52000000LL, 52019999LL, "6M Repeater Inputs",             BAND_6, TRUE);
    info << FrequencyInfo(52020000LL, 52020000LL, "6M FM Simplex",                  BAND_6, TRUE);
    info << FrequencyInfo(52020001LL, 52039999LL, "6M Repeater Inputs",             BAND_6, TRUE);
    info << FrequencyInfo(52040000LL, 52040000LL, "6M FM Simplex",                  BAND_6, TRUE);
    info << FrequencyInfo(52040001LL, 52479999LL, "6M Repeater Inputs",             BAND_6, TRUE);
    info << FrequencyInfo(52480000LL, 52499999LL, "6M DeadBAND_",                    BAND_6, TRUE);
    info << FrequencyInfo(52500000LL, 52524999LL, "6M Repeater Outputs",            BAND_6, TRUE);
    info << FrequencyInfo(52525000LL, 52525000LL, "6M Primary FM Simplex",          BAND_6, TRUE);
    info << FrequencyInfo(52525001LL, 52539999LL, "6M DeadBAND_",                    BAND_6, TRUE);
    info << FrequencyInfo(52540000LL, 52540000LL, "6M Secondary FM Simplex",        BAND_6, TRUE);
    info << FrequencyInfo(52540001LL, 52979999LL, "6M Repeater Outputs",            BAND_6, TRUE);
    info << FrequencyInfo(52980000LL, 52999999LL, "6M DeadBAND_s",                   BAND_6, TRUE);
    info << FrequencyInfo(53000000LL, 53000000LL, "6M Remote Base FM Spx",          BAND_6, TRUE);
    info << FrequencyInfo(53000001LL, 53019999LL, "6M Repeater Inputs",             BAND_6, TRUE);
    info << FrequencyInfo(53020000LL, 53020000LL, "6M FM Simplex",                  BAND_6, TRUE);
    info << FrequencyInfo(53020001LL, 53479999LL, "6M Repeater Inputs",             BAND_6, TRUE);
    info << FrequencyInfo(53480000LL, 53499999LL, "6M DeadBAND_",                    BAND_6, TRUE);
    info << FrequencyInfo(53500000LL, 53519999LL, "6M Repeater Outputs",            BAND_6, TRUE);
    info << FrequencyInfo(53520000LL, 53520000LL, "6M FM Simplex",                  BAND_6, TRUE);
    info << FrequencyInfo(53520001LL, 53899999LL, "6M Repeater Outputs",            BAND_6, TRUE);
    info << FrequencyInfo(53900000LL, 53900000LL, "6M FM Simplex",                  BAND_6, TRUE);
    info << FrequencyInfo(53900010, 53979999LL, "6M Repeater Outputs",            BAND_6, TRUE);
    info << FrequencyInfo(53980000LL, 53999999LL, "6M DeadBAND_",                    BAND_6, TRUE);

    info << FrequencyInfo(144000000LL, 144099999LL, "2M CW",                         -1, TRUE);
    info << FrequencyInfo(144100000LL, 144199999LL, "2M CW/SSB",                     -1, TRUE);
    info << FrequencyInfo(144200000LL, 144200000LL, "2M Calling",                    -1, TRUE);
    info << FrequencyInfo(144200001LL, 144274999LL, "2M CW/SSB",                     -1, TRUE);
    info << FrequencyInfo(144275000LL, 144299999LL, "2M Beacon Sub-BAND_",            -1, TRUE);
    info << FrequencyInfo(144300000LL, 144499999LL, "2M Satellite",                  -1, TRUE);
    info << FrequencyInfo(144500000LL, 144599999LL, "2M Linear Translator Inputs",   -1, TRUE);
    info << FrequencyInfo(144600000LL, 144899999LL, "2M FM Repeater",                -1, TRUE);
    info << FrequencyInfo(144900000LL, 145199999LL, "2M FM Simplex",                 -1, TRUE);
    info << FrequencyInfo(145200000LL, 145499999LL, "2M FM Repeater",                -1, TRUE);
    info << FrequencyInfo(145500000LL, 145799999LL, "2M FM Simplex",                 -1, TRUE);
    info << FrequencyInfo(145800000LL, 145999999LL, "2M Satellite",                  -1, TRUE);
    info << FrequencyInfo(146000000LL, 146399999LL, "2M FM Repeater",                -1, TRUE);
    info << FrequencyInfo(146400000LL, 146609999LL, "2M FM Simplex",                 -1, TRUE);
    info << FrequencyInfo(146610000LL, 147389999LL, "2M FM Repeater",                -1, TRUE);
    info << FrequencyInfo(147390000LL, 147599999LL, "2M FM Simplex",                 -1, TRUE);
    info << FrequencyInfo(147600000LL, 147999999LL, "2M FM Repeater",                -1, TRUE);

    info << FrequencyInfo(222000000LL, 222024999LL, "125M EME/Weak Signal",          -1, TRUE);
    info << FrequencyInfo(222025000LL, 222049999LL, "125M Weak Signal",              -1, TRUE);
    info << FrequencyInfo(222050000LL, 222059999LL, "125M Propagation Beacons",      -1, TRUE);
    info << FrequencyInfo(222060000LL, 222099999LL, "125M Weak Signal",              -1, TRUE);
    info << FrequencyInfo(222100000LL, 222100000LL, "125M SSB/CW Calling",           -1, TRUE);
    info << FrequencyInfo(222100001LL, 222149999LL, "125M Weak Signal CW/SSB",       -1, TRUE);
    info << FrequencyInfo(222150000LL, 222249999LL, "125M Local Option",             -1, TRUE);
    info << FrequencyInfo(222250000LL, 223380000LL, "125M FM Repeater Inputs",       -1, TRUE);
    info << FrequencyInfo(222380001LL, 223399999LL, "125M General",                  -1, TRUE);
    info << FrequencyInfo(223400000LL, 223519999LL, "125M FM Simplex",               -1, TRUE);
    info << FrequencyInfo(223520000LL, 223639999LL, "125M Digital/Packet",           -1, TRUE);
    info << FrequencyInfo(223640000LL, 223700000LL, "125M Links/Control",            -1, TRUE);
    info << FrequencyInfo(223700001LL, 223709999LL, "125M General",                  -1, TRUE);
    info << FrequencyInfo(223710000LL, 223849999LL, "125M Local Option",             -1, TRUE);
    info << FrequencyInfo(223850000LL, 224980000LL, "125M Repeater Outputs",         -1, TRUE);

    info << FrequencyInfo(420000000LL, 425999999LL, "70CM ATV Repeater",             -1, TRUE);
    info << FrequencyInfo(426000000LL, 431999999LL, "70CM ATV Simplex",              -1, TRUE);
    info << FrequencyInfo(432000000LL, 432069999LL, "70CM EME",                      -1, TRUE);
    info << FrequencyInfo(432070000LL, 432099999LL, "70CM Weak Signal CW",           -1, TRUE);
    info << FrequencyInfo(432100000LL, 432100000LL, "70CM Calling Frequency",        -1, TRUE);
    info << FrequencyInfo(432100001LL, 432299999LL, "70CM Mixed Mode Weak Signal",   -1, TRUE);
    info << FrequencyInfo(432300000LL, 432399999LL, "70CM Propagation Beacons",      -1, TRUE);
    info << FrequencyInfo(432400000LL, 432999999LL, "70CM Mixed Mode Weak Signal",   -1, TRUE);
    info << FrequencyInfo(433000000LL, 434999999LL, "70CM Auxillary/Repeater Links", -1, TRUE);
    info << FrequencyInfo(435000000LL, 437999999LL, "70CM Satellite Only",           -1, TRUE);
    info << FrequencyInfo(438000000LL, 441999999LL, "70CM ATV Repeater",             -1, TRUE);
    info << FrequencyInfo(442000000LL, 444999999LL, "70CM Local Repeaters",          -1, TRUE);
    info << FrequencyInfo(445000000LL, 445999999LL, "70CM Local Option",             -1, TRUE);
    info << FrequencyInfo(446000000LL, 446000000LL, "70CM Simplex",                  -1, TRUE);
    info << FrequencyInfo(446000001LL, 446999999LL, "70CM Local Option",             -1, TRUE);
    info << FrequencyInfo(447000000LL, 450000000LL, "70CM Local Repeaters",          -1, TRUE);


    info << FrequencyInfo(902000000LL, 902099999LL, "33CM Weak Signal SSTV/FAX/ACSSB", -1, TRUE);
    info << FrequencyInfo(902100000LL, 902100000LL, "33CM Weak Signal Calling", -1, TRUE);
    info << FrequencyInfo(902100001LL, 902799999LL, "33CM Weak Signal SSTV/FAX/ACSSB", -1, TRUE);
    info << FrequencyInfo(902800000LL, 902999999LL, "33CM Weak Signal EME/CW", -1, TRUE);
    info << FrequencyInfo(903000000LL, 903099999LL, "33CM Digital Modes",   -1, TRUE);
    info << FrequencyInfo(903100000LL, 903100000LL, "33CM Alternate Calling", -1, TRUE);
    info << FrequencyInfo(903100001LL, 905999999LL, "33CM Digital Modes",   -1, TRUE);
    info << FrequencyInfo(906000000LL, 908999999LL, "33CM FM Repeater Inputs", -1, TRUE);
    info << FrequencyInfo(909000000LL, 914999999LL, "33CM ATV",                             -1, TRUE);
    info << FrequencyInfo(915000000LL, 917999999LL, "33CM Digital Modes",   -1, TRUE);
    info << FrequencyInfo(918000000LL, 920999999LL, "33CM FM Repeater Outputs", -1, TRUE);
    info << FrequencyInfo(921000000LL, 926999999LL, "33CM ATV",                             -1, TRUE);
    info << FrequencyInfo(927000000LL, 928000000LL, "33CM FM Simplex/Links", -1, TRUE);

    info << FrequencyInfo(1240000000LL, 1245999999LL, "23CM ATV #1",                -1, TRUE);
    info << FrequencyInfo(1246000000LL, 1251999999LL, "23CM FMN Point/Links", -1, TRUE);
    info << FrequencyInfo(1252000000LL, 1257999999LL, "23CM ATV #2, Digital Modes", -1, TRUE);
    info << FrequencyInfo(1258000000LL, 1259999999LL, "23CM FMN Point/Links", -1, TRUE);
    info << FrequencyInfo(1260000000LL, 1269999999LL, "23CM Sat Uplinks/WideBAND_ Exp", -1, TRUE);
    info << FrequencyInfo(1270000000LL, 1275999999LL, "23CM Repeater Inputs", -1, TRUE);
    info << FrequencyInfo(1276000000LL, 1281999999LL, "23CM ATV #3",                -1, TRUE);
    info << FrequencyInfo(1282000000LL, 1287999999LL, "23CM Repeater Outputs",      -1, TRUE);
    info << FrequencyInfo(1288000000LL, 1293999999LL, "23CM Simplex ATV/WideBAND_ Exp", -1, TRUE);
    info << FrequencyInfo(1294000000LL, 1294499999LL, "23CM Simplex FMN",           -1, TRUE);
    info << FrequencyInfo(1294500000LL, 1294500000LL, "23CM FM Simplex Calling", -1, TRUE);
    info << FrequencyInfo(1294500001LL, 1294999999LL, "23CM Simplex FMN",           -1, TRUE);
    info << FrequencyInfo(1295000000LL, 1295799999LL, "23CM SSTV/FAX/ACSSB/Exp", -1, TRUE);
    info << FrequencyInfo(1295800000LL, 1295999999LL, "23CM EME/CW Expansion",      -1, TRUE);
    info << FrequencyInfo(1296000000LL, 1296049999LL, "23CM EME Exclusive",         -1, TRUE);
    info << FrequencyInfo(1296050000LL, 1296069999LL, "23CM Weak Signal",           -1, TRUE);
    info << FrequencyInfo(1296070000LL, 1296079999LL, "23CM CW Beacons",            -1, TRUE);
    info << FrequencyInfo(1296080000LL, 1296099999LL, "23CM Weak Signal",           -1, TRUE);
    info << FrequencyInfo(1296100000LL, 1296100000LL, "23CM CW/SSB Calling",        -1, TRUE);
    info << FrequencyInfo(1296100001LL, 1296399999LL, "23CM Weak Signal",           -1, TRUE);
    info << FrequencyInfo(1296400000LL, 1296599999LL, "23CM X-BAND_ Translator Input", -1, TRUE);
    info << FrequencyInfo(1296600000LL, 1296799999LL, "23CM X-BAND_ Translator Output", -1, TRUE);
    info << FrequencyInfo(1296800000LL, 1296999999LL, "23CM Experimental Beacons", -1, TRUE);
    info << FrequencyInfo(1297000000LL, 1300000000LL, "23CM Digital Modes",         -1, TRUE);

    info << FrequencyInfo(2300000000LL, 2302999999LL, "23GHz High Data Rate", -1, TRUE);
    info << FrequencyInfo(2303000000LL, 2303499999LL, "23GHz Packet",              -1, TRUE);
    info << FrequencyInfo(2303500000LL, 2303800000LL, "23GHz TTY Packet",  -1, TRUE);
    info << FrequencyInfo(2303800001LL, 2303899999LL, "23GHz General",     -1, TRUE);
    info << FrequencyInfo(2303900000LL, 2303900000LL, "23GHz Packet/TTY/CW/EME", -1, TRUE);
    info << FrequencyInfo(2303900001LL, 2304099999LL, "23GHz CW/EME",              -1, TRUE);
    info << FrequencyInfo(2304100000LL, 2304100000LL, "23GHz Calling Frequency", -1, TRUE);
    info << FrequencyInfo(2304100001LL, 2304199999LL, "23GHz CW/EME/SSB",  -1, TRUE);
    info << FrequencyInfo(2304200000LL, 2304299999LL, "23GHz SSB/SSTV/FAX/Packet AM/Amtor", -1, TRUE);
    info << FrequencyInfo(2304300000LL, 2304319999LL, "23GHz Propagation Beacon Network", -1, TRUE);
    info << FrequencyInfo(2304320000LL, 2304399999LL, "23GHz General Propagation Beacons", -1, TRUE);
    info << FrequencyInfo(2304400000LL, 2304499999LL, "23GHz SSB/SSTV/ACSSB/FAX/Packet AM", -1, TRUE);
    info << FrequencyInfo(2304500000LL, 2304699999LL, "23GHz X-BAND_ Translator Input", -1, TRUE);
    info << FrequencyInfo(2304700000LL, 2304899999LL, "23GHz X-BAND_ Translator Output", -1, TRUE);
    info << FrequencyInfo(2304900000LL, 2304999999LL, "23GHz Experimental Beacons", -1, TRUE);
    info << FrequencyInfo(2305000000LL, 2305199999LL, "23GHz FM Simplex", -1, TRUE);
    info << FrequencyInfo(2305200000LL, 2305200000LL, "23GHz FM Simplex Calling", -1, TRUE);
    info << FrequencyInfo(2305200001LL, 2305999999LL, "23GHz FM Simplex", -1, TRUE);
    info << FrequencyInfo(2306000000LL, 2308999999LL, "23GHz FM Repeaters", -1, TRUE);
    info << FrequencyInfo(2309000000LL, 2310000000LL, "23GHz Control/Aux Links", -1, TRUE);
    info << FrequencyInfo(2390000000LL, 2395999999LL, "23GHz Fast-Scan TV", -1, TRUE);
    info << FrequencyInfo(2396000000LL, 2398999999LL, "23GHz High Rate Data", -1, TRUE);
    info << FrequencyInfo(2399000000LL, 2399499999LL, "23GHz Packet", -1, TRUE);
    info << FrequencyInfo(2399500000LL, 2399999999LL, "23GHz Control/Aux Links", -1, TRUE);
    info << FrequencyInfo(2400000000LL, 2402999999LL, "24GHz Satellite", -1, TRUE);
    info << FrequencyInfo(2403000000LL, 2407999999LL, "24GHz Satellite High-Rate Data", -1, TRUE);
    info << FrequencyInfo(2408000000LL, 2409999999LL, "24GHz Satellite", -1, TRUE);
    info << FrequencyInfo(2410000000LL, 2412999999LL, "24GHz FM Repeaters", -1, TRUE);
    info << FrequencyInfo(2413000000LL, 2417999999LL, "24GHz High-Rate Data", -1, TRUE);
    info << FrequencyInfo(2418000000LL, 2429999999LL, "24GHz Fast-Scan TV", -1, TRUE);
    info << FrequencyInfo(2430000000LL, 2432999999LL, "24GHz Satellite", -1, TRUE);
    info << FrequencyInfo(2433000000LL, 2437999999LL, "24GHz Sat High-Rate Data", -1, TRUE);
    info << FrequencyInfo(2438000000LL, 2450000000LL, "24GHz WideBAND_ FM/FSTV/FMTV", -1, TRUE);

    info << FrequencyInfo(3456000000LL, 3456099999LL, "3.4GHz General", -1, TRUE);
    info << FrequencyInfo(3456100000LL, 3456100000LL, "3.4GHz Calling Frequency", -1, TRUE);
    info << FrequencyInfo(3456100001LL, 3456299999LL, "3.4GHz General", -1, TRUE);
    info << FrequencyInfo(3456300000LL, 3456400000LL, "3.4GHz Propagation Beacons", -1, TRUE);

    info << FrequencyInfo(5760000000LL, 5760099999LL, "5.7GHz General", -1, TRUE);
    info << FrequencyInfo(5760100000LL, 5760100000LL, "5.7GHz Calling Frequency", -1, TRUE);
    info << FrequencyInfo(5760100001LL, 5760299999LL, "5.7GHz General", -1, TRUE);
    info << FrequencyInfo(5760300000LL, 5760400000LL, "5.7GHz Propagation Beacons", -1, TRUE);

    info << FrequencyInfo(10368000000LL, 10368099999LL, "10GHz General", -1, TRUE);
    info << FrequencyInfo(10368100000LL, 10368100000LL, "10GHz Calling Frequency", -1, TRUE);
    info << FrequencyInfo(10368100001LL, 10368400000LL, "10GHz General", -1, TRUE);

    info << FrequencyInfo(24192000000LL, 24192099999LL, "24GHz General", -1, TRUE);
    info << FrequencyInfo(24192100000LL, 24192100000LL, "24GHz Calling Frequency", -1, TRUE);
    info << FrequencyInfo(24192100001LL, 24192400000LL, "24GHz General", -1, TRUE);

    info << FrequencyInfo(47088000000LL, 47088099999LL, "47GHz General", -1, TRUE);
    info << FrequencyInfo(47088100000LL, 47088100000LL, "47GHz Calling Frequency", -1, TRUE);
    info << FrequencyInfo(47088100001LL, 47088400000LL, "47GHz General", -1, TRUE);

    info << FrequencyInfo(0,        0,        "",                               0,     FALSE);




}

FrequencyInfo Frequency::getFrequencyInfo(long long f) {
    FrequencyInfo result=info.at(info.size()-1);

    for(int i=0;i<info.size();i++) {
        result=info.at(i);
        if(result.isFrequency(f)) {
            break;
        }
    }

    return result;
}
