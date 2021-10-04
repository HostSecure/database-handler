#include "databasedatafileparser.h"
#include "databasehandler.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>

//!
//! \brief The parseDeviceProductVendor static function
//! Parses a csv file and populates the productvendor database table
//!
void DatabaseDataFileParser::parseDeviceProductVendor( DatabaseHandler& a_dbHandler )
{
   const char* dataDir = getenv( "HOSTSECURE_DATA_DIR" );
   if( dataDir == nullptr )
   {
      dataDir = ".";
   }

   QString path( dataDir );
   path.append( "/productvendors.txt" );
   QFile file( path );

   if( file.exists() )
   {
      if ( file.open( QIODevice::ReadOnly ) )
      {
         while ( !file.atEnd() )
         {
            QString line = file.readLine();
            QStringList productVendor = line.split(',');

            if( productVendor.size() != 4 )
            {
               qWarning() << "Read incomplete line in productvendor file: " << line;
            }
            else
            {
               a_dbHandler.registerProductVendor(productVendor[0], productVendor[1], productVendor[2], productVendor[3]);
            }
         }
         file.close();
      }
      else
      {
         qCritical() << "Failed to open file "
                     << QFileInfo( file ).absoluteFilePath()
                     << " while adding productvendor data: "
                     << file.errorString();
      }
   }
   else
   {
      qCritical() << "Could not find a productvendor file. Expected to find one here: "
                  << QFileInfo(file).absoluteFilePath();
   }
}

//!
//! \brief The parseDeviceProductVendor static function
//! Parses a csv file and populates the virushash database table
//!
void DatabaseDataFileParser::parseVirusHash( DatabaseHandler& a_dbHandler )
{
   const char* dataDir = getenv("HOSTSECURE_DATA_DIR");
   if(dataDir == nullptr)
   {
      dataDir = ".";
   }

   QString path(dataDir);
   path.append("/virushashes.txt");

   QFile file(path);
   if(file.exists())
   {
      if ( file.open( QIODevice::ReadOnly ) )
      {
         while ( !file.atEnd() )
         {
            QString line = file.readLine();
            QStringList virusHash = line.split(',');
            if(virusHash.size() != 2)
            {
               qWarning() << "Read incomplete line in virushash file: " << line;
            }
            else
            {
               a_dbHandler.registerVirusHash(virusHash[0], virusHash[1]);
            }
         }
         file.close();
      }
      else
      {
         qCritical() << "Failed to open file "
                     << QFileInfo(file).absoluteFilePath()
                     << " while adding virushash data: "
                     << file.errorString();
      }
   }
   else
   {
      qCritical() << "Could not find a virushash file. Expected to find one here: "
                  << QFileInfo(file).absoluteFilePath();
   }
}
