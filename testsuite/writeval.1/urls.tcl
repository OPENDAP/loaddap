
# $Id: urls.tcl,v 1.1 2003/10/22 19:44:32 dan Exp $

# Datasets and their expected output (the information that writeval sends to
# stdout - not the stuff that should be going into the file).

# URI/COADS:
set coads "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/nc/coads_climatology.nc"
set coads_sst_ce "SST\\\[0:0\\\]\\\[10:20\\\]\\\[50:60\\\]"
set coads_dds "Dataset {
    Float64 COADSX\[COADSX = 180\];
    Float64 COADSY\[COADSY = 90\];
    Float64 TIME\[TIME = 12\];
    Grid {
     ARRAY:
        Float32 SST\[TIME = 12\]\[COADSY = 90\]\[COADSX = 180\];
     MAPS:
        Float64 TIME\[TIME = 12\];
        Float64 COADSY\[COADSY = 90\];
        Float64 COADSX\[COADSX = 180\];
    } SST;
    Grid {
     ARRAY:
        Float32 AIRT\[TIME = 12\]\[COADSY = 90\]\[COADSX = 180\];
     MAPS:
        Float64 TIME\[TIME = 12\];
        Float64 COADSY\[COADSY = 90\];
        Float64 COADSX\[COADSX = 180\];
    } AIRT;
    Grid {
     ARRAY:
        Float32 UWND\[TIME = 12\]\[COADSY = 90\]\[COADSX = 180\];
     MAPS:
        Float64 TIME\[TIME = 12\];
        Float64 COADSY\[COADSY = 90\];
        Float64 COADSX\[COADSX = 180\];
    } UWND;
    Grid {
     ARRAY:
        Float32 VWND\[TIME = 12\]\[COADSY = 90\]\[COADSX = 180\];
     MAPS:
        Float64 TIME\[TIME = 12\];
        Float64 COADSY\[COADSY = 90\];
        Float64 COADSX\[COADSX = 180\];
    } VWND;
} coads_climatology.nc;"

# URI/FNOC
set fnoc1 "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/nc/fnoc1.nc"
set fnoc1_ce "u\\\[0:0\\\]\\\[0:9\\\]\\\[0:9\\\]"
set fnoc2 "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/nc/fnoc2.nc"
set fnoc2_ce "u\\\[0:0\\\]\\\[0:9\\\]\\\[0:9\\\]"
set fnoc3 "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/nc/fnoc3.nc"
set fnoc3_ce "u\\\[0:0\\\]\\\[0:9\\\]\\\[0:9\\\],v\\\[0:0\\\]\\\[4:9\\\]\\\[4:9\\\]"

# URI/DSP:
set dsp_1 "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/dsp/f96243170857.img"
set dsp_1_ce "dsp_band_1\\\[20:30\\\]\\\[20:30\\\]"
set dsp_1_dds "Dataset {
    Grid {
     ARRAY:
        Byte dsp_band_1\[lat = 512\]\[lon = 512\];
     MAPS:
        Float64 lat\[lat = 512\];
        Float64 lon\[lon = 512\];
    } dsp_band_1;
    Float64 lat\[lat = 512\];
    Float64 lon\[lon = 512\];
} f96243170857;"

set dsp_2 "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/dsp/east.coast.pvu"
set dsp_2_ce "dsp_band_1\\\[20:30\\\]\\\[20:30\\\]"
set dsp_2_dds "Dataset {
    Byte dsp_band_1\[line = 512\]\[pixel = 512\];
} east;"
#} east.coast.pvu;"

# URI/MatLab:
set nscat_s2 "http://dods.gso.uri.edu/cgi-bin/nph-dsp/data/NSCAT_S2.mat"
set nscat_s2_dds "Dataset {
    Float64 NSCAT_S2\[NSCAT_S2_row = 153\]\[NSCAT_S2_column = 843\];
} NSCAT_S2;"

# JPL/NSCAT:  (I will get valid filenames from Todd soon)
		
#		http://dods.jpl.nasa.gov/cgi-bin/hdf/data/nscat/???

# JGOFS:
set jg_diatoms "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/jg/diatoms"
set jg_diatoms_dds "Dataset {
    Sequence {
        String sta;
        String event;
        String tow;
        String diatom_Nit_bi;
        String diatom_Nit_oth;
        String diatom_Pseudo;
    } Level_0;
} diatoms;"

set jg_ctd "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/jg/ctd"
set jg_ctd_dds "Dataset {
    Sequence {
        String sta;
        String cast;
        String event;
        String year;
        String mon;
        String day;
        String time;
        String lat;
        String lon;
        Sequence {
            String depth;
            String press;
            String temp;
            String cond;
            String sal;
            String potemp;
            String sig_th;
            String sig_t;
            String fluor;
            String beam_cp;
            String par;
        } Level_1;
    } Level_0;
} ctd;"

set jg_bot "http://dodsdev.gso.uri.edu/cgi-bin/dods-3.2/nph-dods/data/jg/bot"
set jg_bot_dds "Dataset {
    Sequence {
        String event;
        String cast;
        Sequence {
            String qflag;
            String sta;
            String bot;
            String depth;
            String press;
            String pressbin;
            String ctdtemp;
            String ctdsal;
            String ctdsig_t;
            String botox;
            String botsal;
            String ctdpotemp;
            String ctdsig_th;
        } Level_1;
    } Level_0;
} bot;"



