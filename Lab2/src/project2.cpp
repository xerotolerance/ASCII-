//
// Created by CJ on 10/10/2016.
//
#include <iostream>
#include <cstring>
#include <fstream>
#include <map>
#include <vector>
#include "p2lex.h"
using namespace std;
map<TokenType , int> num_each_token;                    //dictionary linking TokenTypes of generated tokens to the # of times they occurred
map<TokenType, map<string, int>> num_unique_lexes_per_tokentype;    //dictionary matching TokenType of generated tokens to a sub map of the
                                                                    //unique lexeme that generated the Token and the # of times the lexemes appeared.

extern map<TokenType, string> tokenmap;                 //from p2lex.cpp --> map that matches the TokenTypes to their respective string counterparts

map<int, string> lex_toks = {       //maps the ints 0-2 to the strings "id", "str", & "int" respectively
        {0, "id"},                  //NOTE: this dictionary is used as a means
        {1, "str"},                     //to lookup the meaningful tokentype str
        {2, "int"},                     //from a map iterator (see tokenizer(istream&, bool) for context of use...)
};

void print(string sentence, bool printhelp=false){
    /**
     *
     * Print Wrapper for quick, easy, and versitle help displays on the concole
     *
     * accepts: string, bool
     * calls: n/a
     * returns: void
     *
     * **/

    string helpstr = "\n\n\tHelp-- Use '-v' for Verbose mode.  Enter either one(1) or zero(0) filenames to analyze.\n\n\t\t+ Omitting a filename will cause STD::IN to be read.\n\n\t\t+ Invoke '-v' to receive feedback while reading from STD::IN, Otherwise nothing will be displayed initially.";
    cout << sentence;               //print the string passed in
    if (printhelp)                  //if true was also passed, print the help message
        cout << helpstr << endl;
    cout << endl;
}

void tokenizer(istream& stream, bool verbose = false){
    /**
     * Runs getToken() in a loop over the entire file or to ERR token & displays results
     *
     * If VERBOSE then tokens are displayed in the console as they are found.
     *
     * Syntax errors result in an ERR token being generated.  Analysis will end at that token.
     * Successful analysis of the file will result in a (silent) DONE token being generated.
     * Analysis will end at that token.
     *
     * accepts: istream reference, boolean
     * calls: getToken(istream*)
     * returns: void
     *
     * */

    //Commented out formatting to match grading script's style... (sadface)
    /**verbose ?  print("**In Verbose mode\n-----------------\n") : print("");         //if VERBOSE print verbose heaing**/

    Token t;           //creates empty token to be populated in while loop
    do{                     //do while is useful in getting the first token for t
                            //then checking to see if more tokens can be made before looping

        t = getToken(&stream);  //use the stream passed in to generate the a Token from the next word/phrase on the line (NOTE: alters the stream)

        switch (t.getTok()){
            case ID:
            case STR:       //if the Token is of TokenType ID,STR, or INT...
            case INT:
                num_unique_lexes_per_tokentype[t.getTok()][t.getLexeme()]++;    //increase the count of times a specific lexeme has been encountered, if no count existed previously it is created, set to 0, and then inccreased
                break;
            default:;
        }
        if (t.getTok() != DONE)
            num_each_token[t.getTok()]++;                       //increase the count of times a token of TokenType has been generated as long as the token is not DONE

        if ((verbose && t.getTok() != DONE) || t.getTok() == ERR)
            cout << t << endl;                                  //ONLY display tokens if VERBOSE or an ERR occurred

        /*if (t.getTok() == SC)                                 //<--- unimplemented design choice for aesthetic and readability, gomen...
            cout << endl;*/

    }while( !(t.getTok() == ERR || t.getTok() ==DONE) );        //repeat until a syntax error occurs or the entire file has been analyzed

    if (t.getTok() == DONE) {                               //ONLY generate the token summary after entire file has been successfully analyzed
        /** Formatting omitted to match grading script's style t-t
        if (verbose)
            cout << "\n-----------------" << endl;                                              // if VERBOSE print out a seperation line between tokens and token summary
        cout << endl;
        cout << "lines read: " << linenum << endl << endl;
        cout << "Tokens Found:" << endl;
        */

        for (auto it = num_each_token.begin(); it != num_each_token.end(); it++) {
            cout << /**"\t" <<*/ tokenmap[it->first] << ": " << it->second << endl;                      //print the string representation of the TokenType followed by the number of times a token of TokenType was generated
        }
        /**             ^...again unimplemented blah blah (i'm salty because output looks better with this stuff kept in //.-)
        cout << "\n# Unique Lexemes:" << endl;
        */

        int  count[3] = {0,0,0};
        for (auto it = num_unique_lexes_per_tokentype.begin(); it != num_unique_lexes_per_tokentype.end(); it++)
            count[it->first] = it->second.size();                   //overwrite the zeros in the array with the actual counts for every lex_tok seen, NOTE this leaves a 0 in if zero of a token were generated
        for (int i = 0; i < lex_toks.size(); i++)
            cout << "Number of unique lexemes for " << lex_toks[i] << ": " << count[i] << endl;               //print out # unique lexemes corresponding to each lexeme-storing
    }
}

void ifstreamAnalysis(ifstream& stre, bool verbose = false){
    /**
     * Performs intermediary check to make sure file is open before attempting
     * to analyze the file.
     *
     * accepts: istream reference, boolean
     * calls: tokenizer(istream&, bool)
     * returns: void
     *
     * */

    istream& stream = stre;         //an in-stream of some sort (either std::cin or an ifstream of the user specified file)
    if (stre.is_open())                 //check whether file / stream was found and is open
        tokenizer(stream, verbose);         //runs getToken in a loop over the entire file or to ERR token & displays results
    else
        print("Err -- Specified file not open / not found.", true);      // if the filename was bad or there was a problem opening the file alert the user
}

int main(int argc, char ** argv) {      // main function runs the program & determines the mode of execution
    /**
     * CS 280 - Project 2 - Lexical Analyzer
     *
     * Main for program that analyzes text files based on a specified set of categorizations
     *
     * accepts: [at most] 2 command line args: a MODE & a FILENAME
     * calls: ifstreamAnalysis(ifstream&, bool)
     * returns: 0 upon success
     *
     * */
    ifstream stre;                          // an in-file-stream to deal with commandline inputted filenames

    switch (argc) {
        case 1:
            tokenizer(std::cin);            //if no commandline args are passed, default to analyzing std::cin until user enters
            break;                                  // something constituting a syntax error, NOTE: the DONE token is never achieved
        case 2:
            strcmp(argv[1], "-v") != 0 ? stre.open(argv[1]), ifstreamAnalysis(stre) : tokenizer(std::cin, true);      //if the user enters a single command line arg check whether its '-v',
            break;                                                                                                       //if not, treat the arg as a filename and attempt to open & analyze said file
        case 3:
            strcmp(argv[1], "-v") == 0 ? stre.open(argv[2]), ifstreamAnalysis(stre, true) : print(                  //if user enters 2 command line args, check if the first one is -v, if so treat the second as
                    "Err -- Argument Mismatch\t Try '-v' [filename]", true);                                       //a filename, and attempt to open & analyze that file in VERBOSE mode. Else, notify user of bad syntax
            break;
        default:    print("Too many Arguments.", true);                                                 //program takes a maximum of two commandline args; display help message to user if they enter more than two args
    }

    return 0;                       //if successful run, return error code 0 - success
}



