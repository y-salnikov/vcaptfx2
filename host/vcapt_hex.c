#include <stdio.h>  /* For NULL... */

const char *vcapt_firmware[]={
	":040000000201B9320E",
	":01000B0032C2",
	":0100130032BA",
	":01001B0032B2",
	":0100230032AA",
	":01002B0032A2",
	":01003300329A",
	":01003B003292",
	":01004300328A",
	":01004B003282",
	":01005300327A",
	":01005B003272",
	":01006300326A",
	":03006B0002037D10",
	":030073000203A4E1",
	":03007B000206E397",
	":03008300020E2347",
	":03008B000203C5A8",
	":0300930002041450",
	":03009B000206E07A",
	":0300A300020E4FFB",
	":0300AB00020E50F2",
	":0300B300020E51E9",
	":0300BB00020E54DE",
	":0300C300020E2505",
	":0300CB00020E41E1",
	":0300D300020E42D8",
	":0300DB00020E44CE",
	":0300E300020A3ED0",
	":0300EB00020ECD35",
	":0300F3000206E121",
	":0300FB000206E218",
	":03010300020A3DB0",
	":03010B00020A3FA6",
	":03011300020A409D",
	":03011B00020EB021",
	":03012300020E4D7C",
	":03012B00020E526F",
	":03013300020E5564",
	":03013B00020E575A",
	":03014300020E4E5B",
	":03014B00020E534E",
	":03015300020E5643",
	":03015B00020E5839",
	":03016300020EAFDA",
	":03016B00020E453C",
	":03017300020E4732",
	":03017B00020E4928",
	":03018300020E4B1E",
	":03018B00020E461B",
	":03019300020E4811",
	":03019B00020E4A07",
	":0301A300020E4CFD",
	":0301AB00020E43FE",
	":0301B300020E2415",
	":030224000201B61E",
	":0301B6000204F64A",
	":2002270090E61274E8F00000000090E611E0FF747F5FF00000000090E610E0FF747F5FF094",
	":200247000000000090E613E0FF747F5FF00000000090E614E0FF747F5FF00000000090E6CC",
	":2002670015E0FF747F5FF00000000090E6047480F00000000090E6047402F0000000009073",
	":20028700E604E4F00000000090E6187408F00000000090E6207402F00000000090E621E428",
	":2002A700F00000000090E6D27402F0000000002290E740E4F090E74175F0007463120E26B2",
	":2002C70090E68AE4F090E68B7402F02290E60AE090E740F090E68AE4F090E68B04F022AF19",
	":2002E70082BFB0028015BFB1028005BFB21B80128F1090E68BE4F0758201221202B775820A",
	":2003070001221202D37582012275820022E582FF600475820022AD22AE23AF248D828E8323",
	":200327008FF0E4120E2675820122E582FF7004E52260047582002290E6837412F0E0FF74D3",
	":20034700204FF090E6047480F00000000090E6047402F00000000090E604E4F000000000AB",
	":200367007582012275820122AF82BF01047F0180027F008F8222C0E0C082C083C007C0D01D",
	":2003870075D000D200AF9174EF5FF59190E65D7401F0D0D0D007D083D082D0E032C0E0C021",
	":2003A70082C083C0D075D008AF9174EF5FF59190E65D7402F0D0D0D083D082D0E032C021CB",
	":2003C700C0E0C0F0C082C083C007C006C005C004C003C002C001C000C0D075D00075820059",
	":2003E700120D0FAF9174EF5FF59190E65D7410F0D0D0D000D001D002D003D004D005D00694",
	":20040700D007D083D082D0F0D0E0D02132C021C0E0C0F0C082C083C007C006C005C004C03A",
	":2004270003C002C001C000C0D075D000758201120D0FAF9174EF5FF59190E65D7420F0D0C5",
	":20044700D0D000D001D002D003D004D005D006D007D083D082D0F0D0E0D0213290E60B74FC",
	":2004670003F0C20075100090E680E0FF740A4FF09005DC120E5990E680E0FF74F75FF090A0",
	":20048700E600E0FF74E75F4410F0E500D2E890E668E0FF74084FF090E65CE0FF74014FF016",
	":2004A700E0FF74024FF0E0FF74204FF0E0FF74104FF0D2AF12022702058B300005120A416D",
	":2004C700C200E510602674B1B5101E90E6A0E0FF20E11990E68BE0FFBF030990E74075F0FA",
	":1704E7000012064A75100080037510000206A51204631204C180FB97",
	":2004FE0075AF03759AE4759B007E007F0090E67BE4F00EBE00010FC3EE9480EF648094806A",
	":20051E0040EB2290E6F3E4F090E6C3F090E6C1F090E6C2F090E6C074E4F090E6F4E4F0906F",
	":20053E00E6D3F02290E67174FFF0F5B490E672E0FF74804FF043B6800000000090E6C5E44D",
	":20055E00F00000000090E6C4E4F02290E6C6E4F090E6C7F090E6C8F090E6C9F090E6CAF093",
	":20057E0090E6CBF090E6CCF090E6CDF02290E601745EF090E6F574FFF01205211204FE1240",
	":20059E000542120569C2012285822385832485F025AD23AE24AF258D828E838FF0E5221273",
	":2005BE000E2674082DFAE43EFB8F048A828B838CF0E4120E2674102DFAE43EFB8F048A826F",
	":2005DE008B838CF0E4120E2674182DFDE43EFE8D828E838FF0E4020E2685822285832385E1",
	":2005FE00F024AD22AE23AF248D828E838FF074BA120E2674082DFAE43EFB8F048A828B83D6",
	":20061E008CF07403120E2674102DFAE43EFB8F048A828B838CF0E4120E2674182DFDE43E90",
	":20063E00FE8D828E838FF07436020E26AD82AE83AFF0E5BB30E7FB8D828E838FF0120EB1FE",
	":20065E00FD30E50890E6187409F0800690E6187408F00000000075220090E40075F0001265",
	":20067E0005A675220090E40175F0001205A690E40275F0001205F7900001120909758200EE",
	":20069E00120A26D201D322300137E5BB30E73290E6047480F00000000090E618E4F0000021",
	":2006BE00000090E6047402F00000000090E6187408F00000000090E604E4F000000000C232",
	":0206DE000122F7",
	":201E000012010002FFFFFF40FFFF48200100010200010A060002FFFFFF4001000902190091",
	":151E200001010080320904000001FFFFFF00070582020002005C",
	":191E36000902190001010080320904000001FFFFFF0007058202400000E0",
	":201E500004030904160379002E00730061006C006E0069006B006F0076001C03660078003A",
	":181E700032006C006100660077002D0076006300610070007400000033",
	":0106E00032E7",
	":0106E10032E6",
	":0106E20032E5",
	":0106E30032E4",
	":2006E40085822585832685F02790E601E0FC74FC5CF0E0FC74024CF090E6F574FFF0AA22C9",
	":20070400AB23AC248A828B838CF0120EB190E6F3F074012AF8E43BF98C07888289838FF0A0",
	":20072400120EB1F890E6C3F074022AFDE43BFE8C078D828E838FF0120EB190E6C1F0740368",
	":200744002AFDE43BFE8C078D828E838FF0120EB190E6C2F074052AFDE43BFE8C078D828E39",
	":20076400838FF0120EB190E6C0F074062AFAE43BFB8A828B838CF0120EB1FA90E6F4F07594",
	":20078400AF07AD25AE268E078F9A7E008D9B759DE48E9E7F0090E67BE0FE90E67CF00FBF75",
	":2007A400800040F190E67174FFF0F5B490E672E0FF74804FF043B6800000000090E6C4E400",
	":2007C400F00000000090E6C5E4F090E6C6F090E6C7F090E6C8F090E6C9F090E6CAF090E6BA",
	":2007E400CBF090E6CCF090E6CDF022AD82AE83AFF0E52275F009A4FC2DF9E43EFA8F038942",
	":20080400828A838BF0120EB190E6C6F07B0074012CF9E43BFAE92DF9EA3EF88F028982884C",
	":20082400838AF0120EB190E6C7F074022CF9E43BFAE92DF9EA3EF88F02898288838AF012A3",
	":200844000EB190E6C8F074032CF9E43BFAE92DF9EA3EF88F02898288838AF0120EB190E65B",
	":20086400C9F074042CF9E43BFAE92DF9EA3EF88F02898288838AF0120EB190E6CAF074053B",
	":200884002CF9E43BFAE92DF9EA3EF88F02898288838AF0120EB190E6CBF074062CF9E43B06",
	":2008A400FAE92DF9EA3EF88F02898288838AF0120EB190E6CCF074072CFCE43BFBEC2DFD14",
	":2008C400EB3EFE8D828E838FF0120EB190E6CDF022AC82AD83AEF0FF8E028F038B0190E609",
	":2008E400CEE9F0000000007B0090E6CFEAF0000000007E008D0790E6D0EFF00000000090EC",
	":20090400E6D1ECF022AE82AF838F0590E6D0EDF00000000090E6D1EEF02285822485832566",
	":2009240085F026E5BB30E7FB90E6F1E0FC7B008524828525838526F0EC120E26A3EB120ED5",
	":2009440026E52224FFFBE52334FFFC7A008A007900C3E89522E99523506EE5BB30E7FBEA37",
	":2009640075F002A42524F8E52535F0F9AF26AEBD7D00888289838FF0ED120E26A3EE120EC9",
	":2009840026882789288F29888289838FF0120EB1FEA3120EB1FF8A017D00E9B50308EDB5EB",
	":2009A4000404ADBF8002ADBE8D017D00E94206ED42078527828528838529F0EE120E26A38D",
	":2009C400EF120E260A0209512285822485832585F0267C008C027B00C3EA9522EB95235087",
	":2009E40040E5BB30E7FBEC75F002A42524F9E52535F0FAAB2689828A838BF0120EB1A312B5",
	":200A04000EB1FFF890E6F0E8F089828A838BF0120EB1F9A3120EB17A0090E6F1E9F00C80C7",
	":190A2400B322AF82E5BB30E7FB74044FF5BB22AF82E5BB30E7FB8FBB2219",
	":010A3D003286",
	":010A3E003285",
	":010A3F003284",
	":010A40003283",
	":0601E800E478FFF6D8FDEB",
	":1202120075150075160075173675181E75191C751A1E01",
	":200A410090E6B9E0FF24F45003020B3DEF240A83F582EF241083F583E47373893D9F3D3DF4",
	":200A6100B53DBBD2EA1E0A0A0B0A0B0B0A0B0A0A0A0B120BB7E5826003020B5390E6A0E07D",
	":200A8100FF74014FF0020B53120C40E5826003020B5390E6A0E0FF74014FF0020B53120C93",
	":200AA1008EE5826003020B5390E6A0E0FF74014FF0020B53120D3A020B5312036BAF82907A",
	":200AC100E740EFF090E68AE4F090E68B04F0020B5390E6BAE0F58212036FE582707490E61A",
	":200AE100A0E0FF74014FF0806990E6BCE0FF7522147523007524408F82120314E582700B90",
	":200B010090E6A0E0FF74014FF0804790E740E514F090E68AE4F090E68B04F0803590E6BC14",
	":200B2100E0FF90E6BAE0F5228F82120331E582702190E6A0E0FF74014FF0801690E6B9E011",
	":200B4100F5821202E6E582700990E6A0E0FF74014FF090E6A0E0FF74804FF022AF828F068A",
	":200B610053067FEE24F75003020BB3EE240A83F582EE240D83F583E4738C90A3B3A7B3AB82",
	":200B8100B3AF0B0B0B0B0B0B0B0B0B90E6A022EF30E7067EA27FE680047EA17FE68E828F1F",
	":200BA100832290E6A32290E6A42290E6A52290E6A6229000002290E6B8E0FFBF8002801FFE",
	":200BC100BF81028005BF826F803490E740E4F090E741F090E68AF090E68B7402F0805CE59E",
	":200BE1001625E0FFE515420790E740EFF090E741E4F090E68AF090E68B7402F0803D90E6EB",
	":200C0100BCE0F582120B5DAE82AF83EE4F7003F582228E828F83E0FE30E0047F0180027F06",
	":200C21000090E740EFF090E741E4F090E68AF090E68B7402F08004758200227582012290F3",
	":200C4100E6B8E0FF6005BF0237801190E6BAE0FFBF010575160080317582002290E6BAE0EF",
	":200C6100701A90E6BCE0F582120B5DAE82AF83E0FD5305FE8E828F83EDF0800D75820022AC",
	":200C810090E6B9E0F5820202E67582012290E6B8E0FF6005BF026A801B90E6BAE0FFBF02C1",
	":200CA10002806790E6BAE0FFBF0105751601805A7582002290E6BAE0704390E6BCE0F582AB",
	":200CC100120B5DAE82AF83EE4F7003F582228E828F83E0FD4305018E828F83EDF090E6BC75",
	":200CE100E0FF53070F90E6BCE05480C423541FFE90E6832FF0E0FF74204FF0800D7582001F",
	":200D01002290E6B9E0F5820202E675820122AF82D20210AF02C202EF600E75171C75181EEC",
	":200D2100751936751A1E800C75173675181E75191C751A1EA20292AF2290E6BBE0FFBF011A",
	":200D4100028019BF02028029BF03028033BF0603020DF5BF0703020E0A020E197E007F1E21",
	":200D61008F0690E6B3EEF07E007F1E90E6B4EEF022AF1890E6B3EFF0AE1790E6B4EEF022D3",
	":200D81007D507E1E7F8090E6BAE0F5227B008B020BEAB5220280428D008E018F028D828EE1",
	":200DA100838FF0120EB128F8E439F9880589068A0774012DF9E43EFA8F0489828A838CF09E",
	":200DC100120EB1F9B9030280067D007E007F00ED4E6006C3E5229B50B5ED4E600D8E0790B2",
	":200DE100E6B3EFF090E6B4EDF02290E6A0E0FF74014FF0227E127F1E8F0690E6B3EEF07E3F",
	":200E0100127F1E90E6B4EEF022AF1A90E6B3EFF0AE1990E6B4EEF02290E6A0E0FF74014F7D",
	":020E2100F022BD",
	":2001C6007900E94400601B7A00900ED2780075921CE493F2A308B800020592D9F4DAF27500",
	":0201E60092FF86",
	":010E2300329C",
	":010E2400329B",
	":010E2500329A",
	":1B0E260020F71130F6138883A88220F509F6A8837583002280FEF280F5F022CB",
	":010E4100327E",
	":010E4200327D",
	":010E4300327C",
	":010E4400327B",
	":03004300021F0099",
	":201F000002037D000203A4000206E300020E23000203C500020414000206E000020E4E004E",
	":201F2000020E4F00020E5000020E5100020E5400020E2500020E4100020E4200020E4400F1",
	":201F4000020A3E00020E4E00020ECD000206E1000206E200020A3D00020A3F00020A400049",
	":201F6000020EB000020E4E00020E4E00020E4E00020E4D00020E5200020E5500020E5700FC",
	":201F8000020E5300020E5600020E5800020EAF00020E4500020E4700020E4900020E4B00F1",
	":181FA000020E4600020E4800020E4A00020E4C00020E4300020E24003E",
	":010E4500327A",
	":010E46003279",
	":2001EE007800E84400600A790075921CE4F309D8FC7800E84400600C7900901C00E4F0A3E7",
	":04020E00D8FCD9FA45",
	":010E47003278",
	":010E48003277",
	":010E49003276",
	":010E4A003275",
	":010E4B003274",
	":010E4C003273",
	":010E4D003272",
	":010E4E003271",
	":010E4F003270",
	":010E5000326F",
	":010E5100326E",
	":010E5200326D",
	":010E5300326C",
	":010E5400326B",
	":010E5500326A",
	":010E56003269",
	":010E57003268",
	":010E58003267",
	":200E5900AE82AF8390E600E05418C423541FFDB40100E433FD60067CB17D00801C90E60013",
	":200E7900E05418C423541FFBBB01067A617B0180047AC27B028A048B058C228D231522743B",
	":160E9900FFB522021523E522452370F11EBEFF011FEE4F70E422B5",
	":010EAF003210",
	":010EB000320F",
	":0D01B900758129120ECEE58260030201B6A9",
	":1C0EB10020F71430F6148883A88220F507E6A88375830022E280F7E49322E02250",
	":010ECD0032F2",
	":040ECE007582002207",
	":00000001FF",
	NULL
};
