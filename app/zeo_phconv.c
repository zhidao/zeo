#include <zeo/zeo.h>

enum{
  PHCONV_INPUTFILE = 0,
  PHCONV_OUTPUTFILE,
  PHCONV_ASCII,
  PHCONV_BINARY,
  PHCONV_HELP,
  PHCONV_INVALID
};
zOption option[] = {
  { "i", "in",    "<input file>",  "input STL/PLY/ZTK file", NULL, false },
  { "o", "out",   "<output file>", "output STL/PLY/ZTK file", NULL, false },
  { "a", "ascii", NULL,            "output ASCII STL/PLY file", NULL, false },
  { "b", "bin",   NULL,            "output binary STL/PLY file", NULL, false },
  { "h", "help",  NULL,            "show this message", NULL, false },
  { NULL, NULL, NULL, NULL, NULL, false },
};

void phconv_usage(char *arg)
{
  eprintf( "Usage: %s [options] <input file> <output file>", arg );
  zOptionHelp( option );
  exit( EXIT_SUCCESS );
}

bool phconv_cmdarg(int argc, char *argv[])
{
  zStrAddrList arglist;
  char *srcfile, *dstfile;

  if( argc <= 1 ) phconv_usage( argv[0] );
  zOptionRead( option, argv+1, &arglist );
  if( option[PHCONV_HELP].flag ) phconv_usage( argv[0] );
  zStrListGetPtr( &arglist, 2, &srcfile, &dstfile );
  if( srcfile ){
    option[PHCONV_INPUTFILE].flag = true;
    option[PHCONV_INPUTFILE].arg  = srcfile;
  }
  if( dstfile ){
    option[PHCONV_OUTPUTFILE].flag = true;
    option[PHCONV_OUTPUTFILE].arg  = dstfile;
  }

  if( !option[PHCONV_INPUTFILE].flag ){
    eprintf( "input file not specified.\n" );
    exit( EXIT_FAILURE );
  }
  if( !option[PHCONV_OUTPUTFILE].flag ){
    eprintf( "output file not specified.\n" );
    exit( EXIT_FAILURE );
  }
  zStrAddrListDestroy( &arglist );
  return true;
}

bool phconv_read_stl(zShape3D *shape)
{
  FILE *fin;
  char buf[BUFSIZ];
  bool ret = true;

  if( !( fin = fopen( option[PHCONV_INPUTFILE].arg, "r" ) ) ){
    ZOPENERROR( option[PHCONV_INPUTFILE].arg );
    return false;
  }
  zShape3DInit( shape );
  shape->com = &zeo_shape3d_ph_com;
  eprintf( "read STL file.\n" );
  if( !( ret = zPH3DFReadSTL( fin, zShape3DPH(shape), buf, BUFSIZ ) ? true : false ) )
    eprintf( "read failure.\n" );
  else if( !zNameSet( shape, buf ) ){
    ZALLOCERROR();
    ret = false;
  }
  fclose( fin );
  return ret;
}

bool phconv_read_ply(zShape3D *shape)
{
  FILE *fin;
  bool ret = true;

  if( !( fin = fopen( option[PHCONV_INPUTFILE].arg, "r" ) ) ){
    ZOPENERROR( option[PHCONV_INPUTFILE].arg );
    return false;
  }
  zShape3DInit( shape );
  shape->com = &zeo_shape3d_ph_com;
  eprintf( "read PLY file.\n" );
  if( !( ret = zPH3DFReadPLY( fin, zShape3DPH(shape) ) ? true : false ) )
    eprintf( "read failure.\n" );
  else if( !zNameSet( shape, "noname" ) ){
    ZALLOCERROR();
    ret = false;
  }
  fclose( fin );
  return ret;
}

bool phconv_read(zShape3D *shape)
{
  char *suffix;

  suffix = zGetSuffix( option[PHCONV_INPUTFILE].arg );
  if( strcmp( suffix, "stl" ) == 0 )
    return phconv_read_stl( shape );
  if( strcmp( suffix, "ply" ) == 0 )
    return phconv_read_ply( shape );
  if( strcmp( suffix, "ztk" ) == 0 ){
    eprintf( "read ZTK file.\n" );
    if( !zShape3DReadZTK( shape, option[PHCONV_INPUTFILE].arg ) ){
      eprintf( "read failure.\n" );
      return false;
    }
    return true;
  }
  eprintf( "unknown file type.\n" );
  return false;
}

bool phconv_write_stl(zShape3D *shape)
{
  FILE *fout;

  if( !( fout = fopen( option[PHCONV_OUTPUTFILE].arg, "w" ) ) ){
    ZOPENERROR( option[PHCONV_OUTPUTFILE].arg );
    return false;
  }
  if( option[PHCONV_BINARY].flag ){
    eprintf( "write binary STL file.\n" );
    zPH3DFWriteSTL_Bin( fout, zShape3DPH(shape), zName(shape) );
  } else{
    eprintf( "write ASCII STL file.\n" );
    zPH3DFWriteSTL_ASCII( fout, zShape3DPH(shape), zName(shape) );
  }
  fclose( fout );
  return true;
}

bool phconv_write_ply(zShape3D *shape)
{
  FILE *fout;

  if( !( fout = fopen( option[PHCONV_OUTPUTFILE].arg, "w" ) ) ){
    ZOPENERROR( option[PHCONV_OUTPUTFILE].arg );
    return false;
  }
  if( option[PHCONV_BINARY].flag ){
    eprintf( "write binary PLY file.\n" );
    zPH3DFWritePLY_Bin( fout, zShape3DPH(shape) );
  } else{
    eprintf( "write ASCII PLY file.\n" );
    zPH3DFWritePLY_ASCII( fout, zShape3DPH(shape) );
  }
  fclose( fout );
  return true;
}

bool phconv_write(zShape3D *shape)
{
  char *suffix;

  suffix = zGetSuffix( option[PHCONV_OUTPUTFILE].arg );
  if( strcmp( suffix, "stl" ) == 0 )
    return phconv_write_stl( shape );
  if( strcmp( suffix, "ply" ) == 0 )
    return phconv_write_ply( shape );
  if( strcmp( suffix, "ztk" ) == 0 ){
    eprintf( "write ZTK file.\n" );
    return zShape3DWriteZTK( shape, option[PHCONV_OUTPUTFILE].arg );
  }
  eprintf( "unknown file type.\n" );
  return false;
}

int main(int argc, char *argv[])
{
  zShape3D shape;

  if( !phconv_cmdarg( argc, argv ) ) return EXIT_FAILURE;
  if( phconv_read( &shape ) )
    phconv_write( &shape );
  zShape3DDestroy( &shape );
  return 0;
}
