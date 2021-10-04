#pragma once

class DatabaseHandler;

//!
//! \brief The DatabaseDataFileParser class
//! The code is based on parsing csv files to make it easy to create your own config files and add your own data
//! In reality, the parser would be configured to parse data from predefined sources, which is most likely not in the csv format.
//!
class DatabaseDataFileParser
{
public:
    static void parseDeviceProductVendor( DatabaseHandler& a_dbHandler );
    static void parseVirusHash( DatabaseHandler& a_dbHandler );

private:
    DatabaseDataFileParser() = default;
};
