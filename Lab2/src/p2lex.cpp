//
// Created by CJ on 10/10/2016.
//
#include <string>
#include <iostream>
#include "p2lex.h"
#include <map>
using namespace std;

int linenum = 0;
enum types{LETTER, NUM, OTHER, QUOTE, SPECIAL};                 //declares the different types of characters, the program will
                                                                  // encounter while analyzing the stream

map<TokenType, string> tokenmap = {                             //map that matches the TokenTypes to their respective string counterparts, (linked to by project2.cpp)
        {ID, "id"},	    // identifier
        {STR, "str"},	// string
        {INT, "int"}, 	// integer constant
        {PLUS, "plus"},                      	// the + operator
        {STAR, "star"},                      	// the * operator
        {LEFTSQ, "leftsq"},		                // the [
        {RIGHTSQ, "rightsq"},	                // the ]
        {PRINT, "print"},                   	// print keyword
        {SET, "set"},                      	    // set keyword
        {SC, "sc"},                            // semicolon
        {LPAREN, "lparen"},                 	// The (
        {RPAREN, "rparen"},             		// the )
        {DONE, "done"},                     	// finished!
        {ERR, "err"}     // an unrecoverable error}
};

ostream& operator<<(ostream& os,  Token const& t){
    /** Defines the behavior of the bitwise left shift operator "<<" when used between an ostream reference (such as std::cout) and a Token object.
     *      Note as we do not intend to alter the Token in anyway, but merely print out a lexical representation of it,
     *         we opt to use a constant reference to the Token object.
     *
     * accepts: ostream reference, Token constant reference
     * calls: Token.getTok(), Token.getLexeme()
     * returns: ostream reference (the same one that was passed in)
     *
     * */

    os << tokenmap[t.getTok()];         //add the string representation of the Token to the stream (will cause printing to console if used with cout as destination)
    switch (t.getTok()){
        case ID:
        case STR:                       //if the TokenType of the Token is an ID, STR, or ERR...
        case INT:
        case ERR:
            os << "(" + t.getLexeme() + ")";        //add the word/phrase that generated the Token to the stream surrounded by parenthesis (will cause printing to console if used with cout as destination)
            break;
        default:;               //else do nothing
    }
    return os;              //return the stream that was passed in
}

TokenType whatAmI(string tokBuilder){
    /** IMPLEMENTS STRING TO TOKENTYPE CHECKING
     *
     *      This method picks the TokenType that best describes the word passed to it and returns that TokenType.
     *
     * accepts: string
     * calls: n/a
     * returns: TokenType
     * */

    map<string, TokenType> tokentype_lookup{                    //operator / keyword lookup table, implemented using a dictionary
            {";", SC},
            {"+", PLUS},
            {"*", STAR},
            {"[", LEFTSQ},
            {"]", RIGHTSQ},
            {"(",LPAREN},
            {")", RPAREN},
            {"print", PRINT},
            {"set", SET}
    };

    if ( tokentype_lookup.count(tokBuilder) > 0 )               //checks to see if word is in the table (is a keyword or operator)
        return tokentype_lookup[tokBuilder];                    //return the matching TokenType for the operator/keyword

                                                                                                            //if the word did not match any operators/keywords,
    else if (tokBuilder.size() >= 2 && tokBuilder[0]== '\"' && tokBuilder[tokBuilder.size()-1]=='\"')       //but instead starts and ends with quotation marks (") and has at least 2 characters (quotations included)
        return STR;                                                                                         //return the TokenType STR for string

    else {                                                          //if the word is not a string nor an operator/keyword,  than it is either an integer or an id...
        /**     State flages initialized to true
         *      - they will be set to false depending on the evaluation
         *        of examination of every character in the word*/

        bool is_int = true;         //used to determine if word is an integer
        bool is_id = true;          //used to determine if word is an identifier

        for (int i = 0; i < tokBuilder.size(); i++) {
            if (!isdigit(tokBuilder[i]))                //check each char in the word, one at a time to see if the char is a digit
                is_int = false;                         //if even one char isn't a digit, than the word isnt an integer
                                                        // set the is_int flag to false

            if (!(isalpha(tokBuilder[i])))              //check each char in the word, one at a time to see if the char is a letter
                is_id = false;                          //if even one char isn't a letter, than the word isn't an identifier
                                                        // set the is_id flag to false
        }

        if (is_int)                 //check the is_int flag - if its true, return TokenType INT
            return INT;
        else if (is_id)             //check the is_id flag - if it's true, return TokenType ID
            return ID;

    }

    return ERR;                     //if this line is reached, the word has been determined not to be an integer, string, identifier, keyword, or operator
                                    //and is therefore not recognized as valid in the language and a TokenType of ERR is returned
}

bool isspecial(char c){
    /**
     *  This method examines a passed char and returns true if the char is an operator in the language
     *          if the char is not an operator, it returns false
     *
     * accepts: char
     * calls: n/a
     * returns: bool
     *
     * */

                           //NOTE THE DOUBLE QUOTE (") HAS INTENTIONALLY BEEN LEFT OF THIS LIST
    switch (c) {           // DUE TO ITS USE IN SIGNIFYING A STRING AND ITS APPEARENCE AS SUCH
        case '*':
        case '+':
        case ';':
        case '[':
        case ']':
        case '(':
        case ')':
            return true;
        default:
            return false;
    }
}

types typeOf(char c){
    /**
     *  This method examines a passed char and returns the "type" of character that best catagorizes the char.
     *
     * accepts: char
     * calls: isalpha(char), isdigit(char), isspecial(char)
     * returns: enum(types)
     *
     * */
    if (isalpha(c))         //if character is a letter return types::LETTER
        return LETTER;
    else if (isdigit(c))    //if character is a digit return types::NUMBER
        return NUM;

    else if (c == '\"')     // DIFFERENT THAN SPECIAL; USED FOR STARTING AND ENDING STRINGS
        return QUOTE;               //if character is a quotation mark (") return types::QUOTE

    else if (isspecial(c))  // ALL OPERATORS RECOGNIZED BY THE LANGUAGE
        return SPECIAL;             //if character is an operator as defined in the language return types::SPECIAL

    return OTHER;           // OTHER normally signifies syntax errors, however more generally, it is a blanket term for non-significant symbols/characters
}

Token getToken(istream* instream){

    /**
     *  This method takes an istream reference, continually pulls characters off of the stream until a distinguable word/phrase is made
     *      determines the type of TokenType of that word/phrase, and returns a Token(TokenType, word).
     *      Note: this method alters the stream passed to, chars pulled off remain off unless put back,
     *          be careful about using this method with shared streams...
     *
     * accepts: istream pointer
     * calls: typeOf(char), whatAmI(string), isspace(char), isspecial(char)
     * returns Token(TokenType, word)
     *
     * */


    /**  Important variables that determine the word/expression to be examined at a given time */
    types curr_type, prev_type;             //hold the type of the current character off the stream and the last character taken off the stream respectively, both are undefined to start...
    string word = "";                       //holds the word/expression to be examined at any given point
    TokenType i_am_a;                       //holds the result of examining the word & is used along side the word to create a resulting Token(TokenType, word)

/**  State Flags initially set to false,
 *      will be changed depending on the analysis of charcters off the stream      */

    bool in_a_string = false;       //boolean flag to determine if the char is part of a string
    bool in_a_comment = false;      //boolean flag to determine if the char is part of a comment
    bool in_parens = false;         //boolean flag to determine if the char is part of a parenthesized expression || NOTE: this functionally is not significant at this time...

    char c;             //will hold each char off the stream, one by one

    int its = 0;            //the integer "its" is used to keep track of the position of the
                            // character in the word as the loop progresses.
                            // it is quite helpful in determining what prev_type should be when
                            // looking at the first letter of a word


                    /**                    BEGIN WHILE LOOP                         */
    while(instream->get(c) ){           //pull the next char off the stream and store it in variable c
        curr_type = typeOf(c);

        if (its == 0)                   //when looking at the first letter in the word default prev_type to be the curr_type
            prev_type =  curr_type;

        if (c == '\n' || (c == '\r' && instream->peek()!='\n'))     //Keeping cross-platform use in mind, increase the linenumber counter when a \n or \r is detected
            ++linenum;                                                  //avoids duplicate addition by only operating on the \n on a system that uses \r\n as line terminator

    /**     Set State Flags                                  */
        if (!in_a_comment && c == '\"') {                                   //Check to see if the comment flag is false, if it is & the character is a quotation mark,
                                                                                    //prepare to either begin or end a quote

            /**     Return a Token for string literal if second quote mark is detected          */
            if (in_a_string) {                                   //~~~THIS IS FOR ENDING A QUOTE~~~~
                                                              //----------------------------------------
                word += c;                                       //include the quote mark in the word
                i_am_a = whatAmI(word);                          //find out if the word is a string
                ++its;                                           //increase the char position
                return Token(i_am_a, word);                      //if successful, return a Token(STR, string)
            }
            else {        //  Set the in a comment flag to true otherwise |   //~~~~THIS IS FOR STARTING A QUOTE~~~~~
                                                                            //-------------------------------------------
                in_a_string = true;                                           //set the string bool flag to true, it will remain true until another quote is encountered
                if (prev_type != QUOTE){                                      //if the previous character was not a quote, put the
                    instream->putback(c);                                      //current character back in the stream to be dealt with next pass
                    i_am_a = whatAmI(word);                                   //Discern the word that comes before the quote mark that was just found
                    ++its;                                                      //increment the position in the word
                    return Token(i_am_a, word);                               //return a Token of whatever the word before the quote mark generated
                }
            }

        }
        else if (!in_a_string && c == '/'  && instream->peek()=='/' )       //sets the in a comment flag if // is detected
            in_a_comment = true;
        else if ( !(in_a_string || in_a_comment) && c == '(')               //sets the parens flag if open parenthesis '(' is detected (virtually useless)
            in_parens = true;
        else if (in_parens && c == ')')                                     //turns off the parens flag if close parenthesis ')' is detected (virtually useless)
            in_parens = false;
        else if (in_a_string && (c == '\n' || (c == '\r' && instream->peek()!='\n')))        //since a string is defined as a set of characters enclosed within quotatation marks *on the same line*
            in_a_string = false;                                                                //set the string flag to false if \n or singular \r is encountered

    /**     Move to the next char w/o recording it if we are in a comment               */
        if (in_a_comment){
            if (c == '\n' || (c == '\r' && instream->peek()!='\n'))             //NOTE: comments can only go upto the newline character therefore
                in_a_comment = !in_a_comment;                                       //when newline is encountered, turn the comment flag off
            ++its;                                                                      //increment the position in the word
            continue;                                                           //move to next character in stream without discerning a word/token
        }
    /**     Creates a word or phrase for analysis by concatonating chars off the stream,
     *          stoppes building the word when either a space is hit or the type of one char to the next changes
     *
     *                                        word
     *                                        -------
     *          ie. Five4                     Five          NOTE:(4 is taken off the stream & used to determine that Five was a word,
     *                                                              4 is then PUT BACK on the stream so that it can be evaluated
     *                                                              as part of the next word)                     */

        if (in_a_string || curr_type == prev_type && !isspace(c) && !isspecial(c) ){        //adds a character to the word if:
            word += c;                                                                          //a.) the string flag is active
            ++its;                                                                              //b.) the character is not whitespace or a special character AND the last thing added
                                                                                                        // is the same type as the character
        }
        else if(word.size() < 1 && isspecial(c)){                                           //returns a Token of w/e special character generated a token as long as
            word += c;                                                                          // there is no word waiting to be analyzed before the special character
            i_am_a = whatAmI(word);
            return Token(i_am_a, word);
        }
        else{                                                                               //Reaching this else implies that the character is not special, but the prev_type does not
                                                                                                // match the type of the current character...
            if (!isspace(c))
                instream->putback(c);                                                       //As long as the character is not whitespace, it must be put back into the stream for proccessing next iteration,
                                                                                                //it has merely been used to determine the end of the previous word this pass...

            if (word.size()>0){                                                             //If there is anything waiting to be turned from a word into a token
                i_am_a = whatAmI(word);                                                         //create and return the token generated from that word
                return Token(i_am_a, word);
            }
            else{                                                                           //If there ISN'T anything waiting to be turned into a word, then
                prev_type = curr_type;                                                          // prepare to move on to the next char in the stream by making
                                                                                                    // prev_type equal to the type of the current character
            }
        }
    }                             /**                       END WHILE LOOP                                             */

    return Token(DONE, "");                  //If this line is reached, there is nothing left in the stream to be analyzed therefore the file has been successfully analyzed & we return the silent Token(DONE, "")
                                             //  NOTE: As a side effect of the stream being empty, the file is also closed at this point.
}

