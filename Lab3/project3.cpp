#include "p2lex.h"
#include <string>
#include <istream>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

/////////
//// this class can be used to represent the parse result
//// the various different items that the parser might recognize can be
//// subclasses of ParseTree
map <string, int> int_vars;
map <string, string> str_vars;
vector<string> errmessages;

enum nodetype {
    PLUSOP,
    STAROP,
    BRACKETOP,
    PRIMINT,
    PRIMSTR,
    PRIMID,
    STMTLST,
    SETOP,
    PRINTOP
};

map <nodetype , string> nodetypenames{
    {PLUSOP, "+"},
    {STAROP, "*"},
    {BRACKETOP, "[STR_CMPLX]"},
    {PRIMINT,"INT"},
    {PRIMSTR, "STR"},
    {PRIMID, "ID"},
    {SETOP, "SET"},
    {PRINTOP, "PRINT"},
    {STMTLST, "<STMT>"}
};


bool	verbose;
map<nodetype , int> opTracker;
extern map<TokenType,string> tokenmap;
bool stored = false;
Token temp;


int linenum = 0;
int globalErrorCount = 0;

/// error handler
void error(string msg)
{
	cout << linenum << ": " << msg << endl;
	++globalErrorCount;
}



class ParseTree {

private:
    int nodeid;
    ParseTree *leftChild;
    ParseTree *rightChild;
    Token iTok;
    int	whichLine;

public:
    nodetype iama;
    static int numnodes;
    ParseTree(ParseTree *left = 0, ParseTree *right = 0) : leftChild(left),rightChild(right) {
        whichLine = linenum;
        numnodes++;
        nodeid = numnodes;
    }
    ParseTree * getLeftChild(){
        return leftChild;
    }
    ParseTree * getRightChild(){
        return rightChild;
    }


    nodetype getType(){
        return iama;
    }
    int onWhichLine() { return whichLine; }
    int getid(){
        return nodeid;
    }
};
int ParseTree::numnodes;


/// function prototypes
ParseTree *Program(istream *in);
ParseTree *StmtList(istream *in);
ParseTree *Stmt(istream *in);
ParseTree *Expr(istream *in);
ParseTree *Term(istream *in);
ParseTree *Primary(istream *in);
ParseTree *String(istream *in);


///  wrapper functions for getToken(istream&)
void putback(Token t){
    if (stored)
        cout << "Err: can't store multiple Tokens..." << endl;
    else {
        temp = t;
        verbose ? cout << "Storing the token " << tokenmap[temp.getTok()] << " with lexeme < " << temp.getLexeme() << " > with highest priority for later use" << endl : cout << "";
        stored = true;
    }
}
Token getNextToken(istream *in){
    if (stored){
        stored = false;
        verbose ? cout << "Retrieved stored token "  << tokenmap[temp.getTok()]<< " with lexeme < " << temp.getLexeme() << " >" << endl : cout <<"";
        return temp;
    }
    Token tok = getToken(in);
    verbose ? cout << "Obtained "  << tokenmap[tok.getTok()] << " with lexeme < " << tok.getLexeme() << " >" << endl : cout << "";
    return tok;
}





//// for example, an integer...
class Integer : public ParseTree {
private:
    Token	iTok;

public:
    Integer(const Token& iTok) : ParseTree(), iTok(iTok) {
        verbose ? cout << "found int(" << getInteger() << ")" << endl : cout << "";
        iama = PRIMINT;
    }
    int	getInteger() { return stoi( iTok.getLexeme() ); }

    
};

class StringObj : public ParseTree {
private:
    Token	iTok;

public:
    //constructors for string class
    StringObj(const Token& iTok) : ParseTree(), iTok(iTok) {iama = PRIMSTR;}
    string	getString() { return  iTok.getLexeme() ; }
    
};

class StmtListObj : public ParseTree {
private:
    Token	iTok;

public:
    StmtListObj(ParseTree *lt, ParseTree *rt) : ParseTree(lt,rt) {iama = STMTLST;}
    
    int	getInteger() { return stoi( iTok.getLexeme() ); }
};

class PlusObj : public ParseTree {
private:
    Token	iTok;

public:
    PlusObj(ParseTree *lt, ParseTree *rt) : ParseTree(lt,rt) {iama=PLUSOP;}
    string	getPlus() { return  iTok.getLexeme() ; }

    
};

class StarObj : public ParseTree {
private:
    Token	iTok;
public:
    StarObj(ParseTree *lt, ParseTree * rt) : ParseTree(lt,rt) {iama = STAROP;}
    string	getStar() { return  iTok.getLexeme() ; }
    
};

class BracketsStrObj : public ParseTree {
private:
    Token	iTok;

public:
    BracketsStrObj(ParseTree *lt = 0, ParseTree *rt = 0) : ParseTree(lt,rt) {iama = BRACKETOP;}
    string	getBrackets() { return "[]";  }
};

class IDObj : public ParseTree {
private:
    Token	iTok;

public:
    IDObj(const Token& iTok) : ParseTree(), iTok(iTok) {
        iama = PRIMID;
    }
    TokenType getTokType() {return iTok.getTok();}
    string	getLex() { return iTok.getLexeme(); }
};

class SetObj : public ParseTree {
private:
	Token	iTok;

public:
	SetObj(IDObj *lt, ParseTree *rt) : ParseTree(lt,rt) {
        iama=SETOP;

        switch (lt->getTokType()){
            case INT:
                int_vars[lt->getLex()]=0; //placeholder for actual value to be implemented later...
            case STR:
                str_vars[lt->getLex()] = "";
        }
    }

	int	getInteger() { return stoi( iTok.getLexeme() ); }
};

class PrintObj : public ParseTree {
private:
	Token	iTok;

public:
	PrintObj(ParseTree *lt) : ParseTree(lt, (ParseTree *)nullptr) {iama=PRINTOP;}
    
	int	getInteger() { return stoi( iTok.getLexeme() ); }
};



ParseTree *Program(istream *in)
{
    verbose ? cout << "Checking if program..." << endl : cout << "";
	ParseTree *result = StmtList(in);
    verbose ? cout << "\t>> StmtList Check Complete <<" << endl : cout << "";

	// make sure there are no more tokens...
	if( getNextToken(in).getTok() != DONE )
		return 0;

	return result;
}


ParseTree *StmtList(istream *in)
{
    verbose ? cout << "\tChecking if StmtList..." << endl : cout << "";
    ParseTree *result = 0;
    if ( result = Stmt(in) ){
        verbose ? cout << "\t\t>> Stmt Check Complete -- Success <<" << endl : cout <<"";
        return new StmtListObj( result, StmtList(in) );
    }
    verbose ? cout << "\t\t>> Stmt Check Complete -- Fail <<" << endl : cout << "";
	return 0;
}


ParseTree *Stmt(istream *in)
{
    verbose ? cout << "\t\tChecking if Stmt..." << endl : cout << "";
	Token t, id;

	t = getNextToken(in);
    ParseTree *result = 0;

    if ( ( t.getTok() == PRINT ) && ( result = Expr(in) )&& ( getNextToken(in).getTok() == SC ) ) {
        verbose ? cout << "\t\t\t>> Expr Check Complete <<" << endl : cout <<"";
        return new PrintObj(result);
    }
	else if( ( t.getTok() == SET ) && ( (id = getNextToken(in)).getTok() == ID) && ( result = Expr(in) ) && ( getNextToken(in).getTok() == SC ) ){
        verbose ? cout << "\t\t\t>> Expr Check Complete <<" << endl : cout <<"";

        return new SetObj( new IDObj(id), result);
    }

	else if (t.getTok() != DONE){
        error("Synax error, invalid statement");
    }
	return 0;
}


ParseTree *Expr(istream *in)
{
    verbose ? cout << "\t\t\tChecking if Expr..." << endl : cout << "";
	Token t;
    ParseTree *left = 0, *right = 0;

    if ( left = Term(in) ) {
        verbose ? cout << "\t\t\t\t>> Term Check Complete <<" << endl : cout << "";
        if (((t = getNextToken(in)).getTok() == PLUS)) {
            if (right = Expr(in)) {
                verbose ? cout << "\t\t\t\t\t>> Expr Check Complete <<" << endl : cout << "";
                return new PlusObj(left, right);
            }
        }
        else {
            putback(t);
            return left;
        }
    }
	return 0;
}


ParseTree *Term(istream *in)
{
    verbose ? cout << "\t\t\t\tChecking if Term..." << endl : cout << "";
	Token t;
	ParseTree *left = 0, *right = 0;

	if ( left = Primary(in) ){
        verbose ? cout << "\t\t\t\t\t>> Primary Check Complete <<" << endl : cout <<"";
        if  (( t = getNextToken(in) ).getTok() == STAR ){
            verbose ? cout << "\t\t\t\tStar found..." << endl : cout <<"";
            if ( right = Term(in) ) {
                verbose ? cout << "\t\t\t\t\t>> Term Check Complete <<" << endl : cout << "";
                return new StarObj(left, right);
            }
        }
        else{
            putback(t);
            return left;
        }

    }

	return 0;
}


ParseTree *Primary(istream *in)
{
    verbose ? cout << "\t\t\t\t\tChecking if Primary..." << endl : cout << "";
	Token t = getToken(in);

	if( t.getTok() == ID ) {
        verbose ? cout << "\t\t\t\t\t\t>> BoB found! Primary: ID was found! <<" << endl : cout << "";
		return new IDObj(t);
	}
	else if( t.getTok() == INT ) {
        verbose ? cout << "\t\t\t\t\t\t>> BoB found! Primary: Integer was found! <<" << endl : cout << "";
		return new Integer(t);
	}
	else if( t.getTok() == STR ) {
		putback(t);
        ParseTree * result = String(in);
        verbose ? cout << "\t\t\t\t\t\t>> String Check Complete <<" << endl : cout << "";
		return result;
	}
	else if( t.getTok() == LPAREN ) {
		ParseTree *ex = Expr(in);
        verbose ? cout << "\t\t\t\t\t\t>> Expr Check Complete <<" << endl : cout << "";
		if( ex == 0 )
			return 0;
		t = getNextToken(in);
		if( t.getTok() != RPAREN ) {
			error("expected right parens");
			return 0;
		}

		return ex;
	}

	return 0;
}


ParseTree *String(istream *in)
{
	Token t = getNextToken(in), compop, op;
	ParseTree *defaultstr = new StringObj(t), *left = 0, *right = 0;

	if ( ( ( op = getNextToken(in) ).getTok() == LEFTSQ) ){
		if (left = Expr(in)){
            verbose ? cout << "\t\t\t\t\t\t\t>> Expr Check Complete <<" << endl : cout << "";
			if ( ( compop = getNextToken(in) ).getTok() == RIGHTSQ ){
                ParseTree * result = new BracketsStrObj(left);
                verbose ? cout << "\t\t\t\t\t\t\t>> BoB found! Primary: Complex String was found! <<" << endl : cout << "";
                return result;
            }

			else if ( ( compop.getTok() == SC ) && ( right = Expr(in) ) && ( getNextToken(in).getTok() == RIGHTSQ ) ){
                verbose ? cout << "\t\t\t\t\t\t\t>> Expr Check Complete <<" << endl : cout << "";
                ParseTree *result = new BracketsStrObj(left, right);
                verbose ? cout << "\t\t\t\t\t\t\t>> BoB found! Primary: Slice String was found! <<" << endl : cout << "";
                return result;
            }
			else
				return  0;
		}
		else
			return 0;
	}

	putback(op);
    verbose ? cout << "\t\t\t\t\t\t\t>> BoB found! Primary: String was found! <<" << endl : cout << "";
	return defaultstr;
}

int transverse(ParseTree * node){

    if (node){
        verbose ? cout << "NODE found!" << endl <<
                "CHK LF - Going down one level... " : cout << "";

        verbose ? cout << "\n\t >> visited node " << node->getid() << "[" << nodetypenames[node->getType()] <<"] <<" << endl : cout << "";
        opTracker[node->getType()]++;

        if (node -> getType() == PRIMSTR && ((StringObj*)node)->getString() == "\"\"")
            errmessages.emplace_back("Empty string not permitted on line " + to_string(node->onWhichLine()) );

        if ( node -> getType() == SETOP ) {
            IDObj * id = (IDObj *)node->getLeftChild();
            verbose ? cout << "Assigning value to [" << id->getLex() << "]" <<endl : cout << "";
            str_vars[id->getLex()] = "fuck me daddy"; //will eventual add evaluateExpr() function instead of this shit
        }
        if (node -> getType() == PRIMID && str_vars.count(((IDObj *)node)->getLex()) < 1)
            errmessages.emplace_back("Symbol " + ((IDObj *)node)->getLex() + " used without being set at line " + to_string(node->onWhichLine()) );

        transverse(node->getLeftChild());

        verbose ? cout << "\n\t >> visited node " << node->getid() << " <<"<< endl <<
                "ELB" << endl <<
                "CHK RT - Going down one level... " : cout << "";
        transverse(node->getRightChild());
        verbose ? cout << "\nERB" << endl : cout << "";

    }
    verbose ? cout << "NODE NOT FOUND" << endl <<
            "EOB - going up one level..." : cout << "";
    return ParseTree::numnodes;
}


int main(int argc, char ** argv){

    istream *in = &cin;
    fstream infile;

    for( int i=1; i<argc; i++ ) {
        if( string(argv[i]) == "-v" )
            verbose = true;
        else {
            if( in != &cin ) {
                cerr << "Too many file names specified" << endl;
                return 1;
            }
            infile.open(argv[i]);
            if( !infile.is_open() ) {
                cerr << "Could not open file named: " << argv[i] << endl;
                return 1;
            }

            in = &infile;
        }
    }

    ParseTree *prog = Program(in);


    verbose ? cout << ">> Program Check Complete <<" << endl : cout << "";
	if( prog == 0 || globalErrorCount != 0 ) {
		verbose ? cout << "Parse failed, exiting" << endl : cout << "";
		return 0;
	}
    verbose ? cout << "Success. Congrats!" << endl : cout << "";

    ParseTree * node = prog;

    int transResult = transverse(prog);

    verbose ? cout << "Total number of nodes in tree: " << transResult << endl : cout << "";
    cout << "Count of + operators: " << opTracker[PLUSOP] << endl;
    cout << "Count of * operators: " << opTracker[STAROP] << endl;
    cout << "Count of [] operators: " << opTracker[BRACKETOP] << endl;

    for (auto err: errmessages)
        cerr << err << endl;

	return 0;
}
