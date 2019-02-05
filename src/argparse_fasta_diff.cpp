
/* argument parsing ------------------------------------------------------------------------------------------------- */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "argparse_fasta_diff.hpp"

// some crazy shit for stringifying preprocessor directives
#define STRIFY(x) #x
#define TO_STR(x) STRIFY(x)

namespace argparse {
    const char usage[] =
    "usage: " PROGNAME " [-h] "
    "[-o OUTPUT] "
    "[-m MASTER] "
    "[-p OPERATION] "
    "[-t MATCH] "
    "[-q] "
    "[FASTA]\n";
    
    const char help_msg[] =
    "Read two FASTA MSA -- a master file and a file containing additional sequences, and output the result of their union/difference\n"
    "\n"
    "optional arguments:\n"
    "  -h, --help               show this help message and exit\n"
    "  -m MASTER                a master FASTA file assumed aligned and with unique sequence IDs \n"
    "  -o OUTPUT                direct the FASTA file with matching (and trimmed reads) to a file named OUTPUT (default=stdout)\n"
    "  -p OPERATION             the output is generated by performing the following operations (default=" TO_STR( DEFAULT_OPERATION ) ")\n"
    "                           add: UNIQUE sequences from FASTA are added to MASTER  \n"
    "                           replace: sequences from FASTA REPLACE sequences with the same ID in MASTER; new sequences are added\n"
    "                           remove: sequences from FASTA are REMOVED from the MASTER if they match \n"
    "  -t MATCH                 sequences in MASTER and FASTA are matched using (default=" TO_STR( DEFAULT_MATCH ) ")\n"
    "                           id: ONLY the sequence ID  \n"
    "                           id_sequence: BOTH sequence ID and the sequence itself (not case sensitive\n"
    "  FASTA                    read sequences to compare from this file (default=stdin)\n";
    
    inline
    void help()
    {
        fprintf( stderr, "%s\n%s", usage, help_msg );
        exit( 1 );
    }
    
    inline
    void ERROR( const char * msg, ... )
    {
        va_list args;
        fprintf( stderr, "%s" PROGNAME ": error: ", usage );
        va_start( args, msg );
        vfprintf( stderr, msg, args );
        va_end( args );
        fprintf( stderr, "\n" );
        exit( 1 );
    }
    
    const char * next_arg (int& i, const int argc, const char * argv[]) {
        i++;
        if (i == argc)
            ERROR ("ran out of command line arguments");
        
        return argv[i];
        
    }
    
    args_t::args_t( int argc, const char * argv[] ) :
    input_master (NULL),
    input_add( stdin ),
    output( stdout ),
    op ( DEFAULT_OPERATION ),
    checks ( DEFAULT_MATCH ),
    quiet (false)
    {
        // skip arg[0], it's just the program name
        for (int i = 1; i < argc; ++i ) {
            const char * arg = argv[i];
            
            if ( arg[0] == '-' && arg[1] == '-' ) {
                if ( !strcmp( &arg[2], "help" ) ) help();
                else
                    ERROR( "unknown argument: %s", arg );
            }
            else if ( arg[0] == '-' ) {
                if ( !strcmp( &arg[1], "h" ) ) help();
                else if (  arg[1] == 'o' ) parse_output( next_arg (i, argc, argv) );
                else if (  arg[1] == 'm')  parse_input_master( next_arg (i, argc, argv) );
                else if (  arg[1] == 'p')  parse_file_operation ( next_arg (i, argc, argv) );
                else if (  arg[1] == 't')  parse_match_mode( next_arg (i, argc, argv) );
                else if (  arg[1] == 'q')  parse_quiet ( );
                else
                    ERROR( "unknown argument: %s", arg );
            }
            else
                if (i == argc-1) {
                    parse_input_add (arg);
                } else {
                    ERROR( "unknown argument: %s", arg );
                }
        }
        
        if (input_master == NULL) {
            ERROR ("Required argument MASTER FILE was not provided");
        }
    }
    
    args_t::~args_t() {
        if ( output && output != stdout )
            fclose( output );
        
        if ( input_add && input_add != stdin)
            fclose (input_add);
        
        if ( input_master )
            fclose (input_master);
        
    }
    
    
    void args_t::parse_output( const char * str ) {
        if ( str && strcmp( str, "-" ) )
            output = fopen( str, "wb" );
        else
            output = stdout;
        
        if ( output == NULL )
            ERROR( "failed to open the OUTPUT file %s", str );
    }
    
    void args_t::parse_input_add ( const char * str ) {
        if ( str && strcmp( str, "-" ) )
            input_add = fopen( str, "rb" );
        else
            input_add = stdin;
        
        if ( input_add == NULL )
            ERROR( "failed to open the INPUT file %s", str );
    }
    
    
    void args_t::parse_input_master ( const char * str )
    {
        if ( str  )
            input_master = fopen( str, "rb" );
        
        if ( input_master == NULL )
            ERROR( "failed to open the MASTER file %s", str );
    }
    
    void args_t::parse_file_operation( const char * str )
    {
        if (!strcmp (str, "add")) {
            op = add;
        } else if (!strcmp (str, "replace")) {
            op = replace;
        } else if (!strcmp (str, "remove")) {
            op = remove;
        } else  {
            ERROR( "invalid file operation: %s", str );
        }
    }
    
    void args_t::parse_match_mode ( const char * str )
    {
        if (!strcmp (str, "id")) {
            checks = id;
        } else if (!strcmp (str, "id_sequence")) {
            checks = id_and_sequence;
        } else  {
            ERROR( "invalid match mode: %s", str );
        }
    }
    
    void args_t::parse_quiet ( void ) {
        quiet = true;
    }
}