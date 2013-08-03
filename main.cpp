//
//  main.cpp
//  SP_1
//
//  Created by 9927262 on 12/10/29.
//  Copyright (c) 2012年 Denny Lin.  All rights reserved.
//
#include <stdexcept>

#include <iostream>
#include <iomanip>

#include <sstream>
#include <fstream>

#include <string>
#include <cstring>
#include <cmath>

#include <vector>

#include <cctype>


using namespace std;

#define TABLE_SIZE 200
#define LINE_TOKEN_MAX 200
#define LIST_SIZE 1000
#define SYM_SIZE 1000

////////
fstream input, output;
///////


// Utilities
int hash( string key ) {
  int sum = 0;
  int i ;
  for ( i = 0 ; i < key.size(); i++ ) {
    sum += key[i] ;
  }
  return sum % TABLE_SIZE ;
}

string lowstr( string s ) {
  int i ;
  
  for ( i = 0 ; i < s.size(); i++ ) {
    if ( isalpha(s[i]) )
      s[i] = tolower(s[i]);
  }
  
  return s ;
  
}

int lowcmp( string s1, string s2 ) {
  return lowstr(s1).compare(lowstr(s2)) ;
}

char RegLT[130];

/////////////////////////


///// Token ///////////

enum TokenType {
  INS, PSE, REG, DELM, SYM, INT, CHAR
  /*
   INTtruction
   PSEudo Instruction
   REGister
   DELiMiter
   SYMbol
   INTeger Constant
   CHARacter Constant
   */
} ;

typedef struct {
  string str;
  TokenType type;

} Token ;

//////// Line //////////////

class Line {
  
public:  
  string mSrc;
  int mloc;
  
  Token mTokens[LINE_TOKEN_MAX];
  int mtop;


  int mType;
  /*
   {
   -1: comment
   0: Pseudo Instruction
   1~4: type 1~4 Instruction
   5: BYTE
   6: WORD
   }
   */
  int mObjCode;
  int mCodeSize;
  
public:
  Line() {
    mSrc = "";
    mloc = -1;
    mtop = -1;
    
    mObjCode = 0;
    mCodeSize = 0; // bytes
  }
  
  void set_src( string src ) {
    mSrc = src ;
  }
  
  void add_token( Token t ) {
    mTokens[++mtop] = t ;
  }
  
  void set_op( int op ) {
    if ( mType >= 1 && mType <= 4 ) {
      mObjCode |= ( op << ((mType-1)*8));
      mCodeSize = mType ;
    }
  }
  
  void set_mask( char n, char b ) {
    int m ;
    if ( b == 1 ) 
    {
      m = (1 << n);
      mObjCode |= m;
      
    } else {
      
      m = (1 << (mCodeSize*8 + 1));
      m -= 1; 
      m -= (1 << n);
      
      mObjCode &= m ;
    }
    
  }
  
  void set_flag( char f, char b ) {
    char flag3[255];
    char flag4[255];
    
    flag3['n'] = 17;
    flag3['i'] = 16;
    flag3['x'] = 15;
    flag3['b'] = 14;
    flag3['p'] = 13;
    flag3['e'] = 12;
    
    flag4['n'] = 25;
    flag4['i'] = 24;
    flag4['x'] = 23;
    flag4['b'] = 22;
    flag4['p'] = 21;
    flag4['e'] = 20;
    
    
    if      ( mType == 3 ) set_mask(flag3[f], b) ;
    else if ( mType == 4 ) set_mask(flag4[f], b) ;
  }
  
  void set_r1( char r1 ) {
    if      ( mType == 2 ) mObjCode |= r1 << 4;
  }
  
  void set_r2( char r2 ) {
    if      ( mType == 2 ) mObjCode |= r2;
  }
};

Line LLTable[LIST_SIZE];
int nowll = -1;

//////////////////////////

////////// Instruction Table ////////////

typedef struct {
  string str;
  int type ;
  int opd_num ;
  int opcode;
} InsDef ;

class InsTab {
  
private:
  int type ;
  
  InsDef t[TABLE_SIZE];
  int top ;
  
public:
  InsTab( TokenType t ) {
    type = t ;
    top = -1;
  }
  
  bool exist( string key ) {
    int i;
    for ( i = 0 ; i <= top ; i++ ) {
      if ( lowcmp(key, t[i].str) == 0 ) {
        return true; 
      }
    }
    return false;
  }
  
  InsDef get( string key ) {
    int i;
    for ( i = 0 ; i <= top ; i++ ) {
      if ( lowcmp(key, t[i].str) == 0 ) {
        return t[i]; 
      }
    }
  }
  
  bool add_to_line( string key ) {
    if ( exist(key) ) {
      Token t ;
      t.str = key;
      t.type = INS ;
      
      LLTable[nowll].add_token( t );
      return true ;
    } else {
      return false ;
    }
  }
  
  void add_file( string filename ) {
    fstream fin;
    fin.open(filename.c_str(), fstream::in) ;
    
    while ( !fin.eof() ) {
      top++;
      fin >> t[top].str >> t[top].type >> t[top].opd_num ;
      fin >> std::hex >> t[top].opcode >> std::dec ;
    }
    
    fin.close();
    
  }
};

class PTab {
  // parsing table with normal lookup usage
  
private:
  TokenType type ;
  
  string t[TABLE_SIZE];
  int top ;
  
public:
  
  PTab( TokenType t ) {
    type = t ;
    top = -1 ;
  }
  
  bool exist( string key ) {
    int i;
    for ( i = 0 ; i <= top ; i++ ) {
      if ( lowcmp(key, t[i]) == 0 ) {
        return true; 
      }
    }
    return false;
  }
  
  bool add_to_line( string key ) {
    
    if ( exist(key) || type == INT || type == CHAR || type == SYM ) {
      Token t ;
      t.str = key;
      t.type = type ;
      
      LLTable[nowll].add_token( t );
      return true ;
    } else {
      return false ;
    }
  }
  
  void add_file( string filename ) {
    fstream fin;
    fin.open(filename.c_str(), fstream::in) ;
    
    while ( !fin.eof() ) {
      top++;
      fin >> t[top] ;
    }
    
    fin.close();
    
  }
};

InsTab InsT = InsTab( INS ) ;
PTab PseT = PTab( PSE ) ;
PTab RegT = PTab( REG ) ;
PTab DelmT = PTab( DELM ) ;
PTab SymT = PTab( SYM ) ;
PTab IntT = PTab( INT );
PTab CharT = PTab( CHAR );

bool add_over( string key ) {
  
  return InsT.add_to_line(key) ||
  PseT.add_to_line(key) ||
  RegT.add_to_line(key) ||
  DelmT.add_to_line(key) ;
  
}


void Lex()
{
  
  string linebuf ;
  int pp ;
  
  
  
  try {
    InsT.add_file("Table1.table");
    PseT.add_file("Table2.table");
    RegT.add_file("Table3.table");
    DelmT.add_file("Table4.table");
    
    
    while ( !input.eof() ) {
      nowll++;
      
      std::getline(input, linebuf);
      
      // output << line ;
      
      LLTable[nowll].set_src(linebuf) ;
      
      linebuf += '\n';
      
      pp = 0;
      while ( pp < linebuf.size()) {
        if ( !isspace(linebuf[pp]) ) {
          
          // ******Hex Constant
          if ( linebuf[pp] == 'X' || linebuf[pp] == 'x' ) {
            if ( linebuf[pp+1] == '\'' ) {
              DelmT.add_to_line("\'");
              pp++;
              pp++;
              
              
              string buf ;
              while ( linebuf[pp] != '\'' ) {
                buf += linebuf[pp];
                pp++;
              }
              
              IntT.add_to_line( buf ) ;
              
              if ( linebuf[pp+1] == '\'' ) {
                DelmT.add_to_line("\'");
                pp++;
              }
            }
          }
          
          // ******Char Constant
          if ( linebuf[pp] == 'C' || linebuf[pp] == 'c' ) {
            if ( linebuf[pp+1] == '\'' ) {
              DelmT.add_to_line("\'");
              pp++;
              pp++;
              
              
              string buf ;
              while ( linebuf[pp] != '\'' ) {
                buf += linebuf[pp];
                pp++;
              }
              
              CharT.add_to_line(buf);
              
              if ( linebuf[pp+1] == '\'' ) {
                DelmT.add_to_line("\'");
                pp++;
              }
            }
          }
          
          // ******Token starts with alphabets
          if ( isalpha( linebuf[pp]) ) {
            string buf ;
            buf += linebuf[pp];
            
            pp++;
            while ( isalpha(linebuf[pp]) || isdigit(linebuf[pp]) ) {
              buf += linebuf[pp];
              pp++;
            }
            
            if ( !add_over(buf) ) {
              SymT.add_to_line(buf);
            }
          
          // ******Token starts with digits
          } else if ( isdigit( linebuf[pp]) ) {
            string buf ;
            buf += linebuf[pp];
            
            pp++;
            while ( isdigit(linebuf[pp]) ) {
              buf += linebuf[pp];
              pp++;
            }
            
            if ( !add_over(buf) ) {
              IntT.add_to_line(buf);
            }
          } else {
            string buf;
            buf += linebuf[pp];
            
            
            if ( buf.compare(".") == 0 ) {
              LLTable[nowll].mType = -1;
              pp = linebuf.size();
            } else {
              add_over(buf);
              pp++;
            }

            
          }
        } else {
          // if ( linebuf[pp] == '\n' ) output << endl;
          pp++;
          
        }
        
      } 
      
    }
    
  } catch (out_of_range & e) {
    output<< "[Error] Assembling failed with errors.";
  } 
  
}

/////// Symbol Table

typedef struct {
  string label;
  int locc ; 
  
} Symbol;

class SymbolTable {
public:
  Symbol s[SYM_SIZE];
  int top;
  
  Symbol sUnknown ;
  
  SymbolTable() {
    top = -1 ;
    
    sUnknown.label = "UNKNOW";
    sUnknown.locc = 0;
  }
  
  bool exist( string key ) {
    int i;
    for ( i = 0 ; i <= top ; i++ ) {
      if ( lowcmp(key, s[i].label) == 0 ) {
        return true; 
      }
    }
    return false;
  }
  
  Symbol& get( string key ) {
    int i;
    for ( i = 0 ; i <= top ; i++ ) {
      if ( lowcmp(key, s[i].label) == 0 ) {
        return s[i]; 
      }
    }
    
    //output << "[!" << key << "]" << endl ;
    return sUnknown;
  }
  
  
  void add( string key, int v ) {
    if ( exist(key) ) {
      return ;
    } else {
      top++;
      s[top].label = key ;
      s[top].locc = v ;
    }
  }
  
  void set( string key, int v ) {
    if ( exist(key) ) {
      Symbol& nows = get(key) ; 
      
      nows.locc = v ;
    } 
  }
  
};

/////////////////

int locc = 0;
int pc = 0;
int rb = 0;

SymbolTable SymbolT ;

void pass1() {
  int i ;
  
  for ( i = 0 ; i < nowll ; i++ ) {
    Line &nowline = LLTable[i] ;
    Token *t = nowline.mTokens ;
    
    int k ;
    
    for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
      
      //INS, PSE, REG, DELM, SYM, INT, CHAR
      
      if ( nowline.mType == -1 ) continue;
      // ignored if the line is comment
      
      
      
      switch ( t[k].type ) {
        case SYM:
          if ( k == 0 ) {
            SymbolT.add( t[k].str, locc ) ;
          }
          break;
          
        case PSE:
          nowline.mType = 0;
          
          if ( lowcmp( t[k].str, "start" ) == 0 ) {
            if ( t[k+1].type == INT ) {
              t[k+1].str = "0x" + t[k+1].str ;
              sscanf( t[k+1].str.c_str(), "%x", &locc);
            }
            nowline.mloc = locc ;
          } else if ( lowcmp( t[k].str, "byte" ) == 0 ) {
            nowline.mType = 5 ;
            
            nowline.mloc = locc ;
            if ( t[k+2].type == CHAR ) {
              locc += t[k+2].str.size();
              nowline.mCodeSize = t[k+2].str.size() ;
            } else if ( t[k+2].type == INT ) {
              locc += t[k+2].str.size()/2;
              nowline.mCodeSize = t[k+2].str.size()/2 ;
            }
            
            
            
          } else if ( lowcmp( t[k].str, "word" ) == 0 ) {
            nowline.mType = 6 ;
            
            nowline.mloc = locc ;
            if ( t[k+2].type == INT ) {
              locc += 3;
              nowline.mCodeSize = 3;
            }
            
          }
          else if ( lowcmp( t[k].str, "resb" ) == 0 ) {
            nowline.mloc = locc ;
            if ( t[k+1].type == INT ) {
              int n ;
              sscanf(t[k+1].str.c_str(), "%d", &n );
              locc += n;
            }
          } else if ( lowcmp( t[k].str, "resw" ) == 0 ) {
            nowline.mloc = locc ;
            if ( t[k+1].type == INT ) {
              int n ;
              sscanf(t[k+1].str.c_str(), "%d", &n );
              locc += n*3;
            }
          } else if ( lowcmp( t[k].str, "equ" ) == 0 ) {   
            if ( t[k+1].type == INT ) {
              sscanf(t[k+1].str.c_str(), "%d", &nowline.mloc );
              SymbolT.set( t[k-1].str, nowline.mloc ) ;
            }
          }
          continue;
          break;
          
        case INS:
          nowline.mloc = locc ;
          InsDef idef = InsT.get(t[k].str);
          if ( idef.type == 3 ) {
            if ( k == 0 ) {
              nowline.mType = idef.type ;
            } else {
              if ( lowcmp( t[k-1].str, "+" ) == 0 ) {
                nowline.mType = 4 ;
              } else {
                nowline.mType = 3 ;
              }

            }
            
          } else {
            nowline.mType = idef.type ;
          }
          
          locc += nowline.mType ;
          
          continue;
          break;
      } // switch
      
    } // for k

  } // for i
  
} // pass1()

void pass2() {
  int i, k, ln ;
  
  
  for ( i = 0 ; i <= nowll ; i++ ) {
    Line &nowline = LLTable[i] ;
    Token *t = nowline.mTokens ;
    InsDef idef;
    
    Symbol opd;
    int addr;
    
    
    switch ( nowline.mType ) {
        
      case -1:
        continue;
      case 0:
        if ( lowcmp( t[0].str, "base" ) == 0 ) {
          if ( t[1].type == INT ) {
            sscanf(t[1].str.c_str(), "%d", &rb) ;
          } else if ( t[1].type == SYM ) {
            Symbol opd = SymbolT.get( t[1].str ) ;
            rb = opd.locc;
          }
        }
        break;
        ////////////////////////////////////
      case 1:
        nowline.mCodeSize = 1;
        
        for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
          if ( t[k].type == INS ) break;
        } 
        
        idef = InsT.get(t[k].str);
        nowline.set_op(idef.opcode);

        break;
        ////////////////////////////////////  
      case 2:
        nowline.mCodeSize = 2;
        
        for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
          if ( t[k].type == INS ) break;
        } 
        
        idef = InsT.get(t[k].str);
        nowline.set_op(idef.opcode);
        
        if ( idef.opd_num >= 1 ) {
          if ( t[k+1].type == REG ) {
            nowline.set_r1(RegLT[t[k+1].str[0]]);
          }
        }
        
        if ( idef.opd_num >= 2 ) {
          if ( t[k+3].type == REG ) {
            nowline.set_r2(RegLT[t[k+3].str[0]]);
          }
        }
        
        break;
        ////////////////////////////////////  
      case 3:
        nowline.mCodeSize = 3;
        
        for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
          if ( t[k].type == INS ) break;
        } 
        
        idef = InsT.get(t[k].str);
        nowline.set_op(idef.opcode);
        
        if ( t[k+1].type == DELM ) {
          if ( lowcmp( t[k+1].str , "#" ) == 0 ) {
            nowline.set_flag('n', 0 );
            nowline.set_flag('i', 1 );
            if ( t[k+2].type == INT ) {
              sscanf(t[k+2].str.c_str(), "%d", &addr) ;
              nowline.mObjCode += addr;
              continue;
            }
          } else if ( lowcmp( t[k+1].str , "@" ) == 0 ) {
            nowline.set_flag('n', 1 );
            nowline.set_flag('i', 0 );
          } 
          k++;
        } else {
          nowline.set_flag('n', 1 );
          nowline.set_flag('i', 1 );
        }
        
        if ( lowcmp(t[k+3].str, "x") == 0 ) {
          nowline.set_flag('x', 1 );
        }
        
        if ( idef.opd_num == 0 ) continue;
        
        pc = -1;
        for ( ln = i+1 ; ln <= nowll ; ln++ ) {
          if ( LLTable[ln].mloc != -1 ) {
            pc = LLTable[ln].mloc ;
            break;
          }
        }
        
        opd = SymbolT.get( t[k+1].str ) ;
        //output << "[%" << t[k].str<<" , "<< t[k+1].str << "]" << endl ;
        
        addr = opd.locc - pc ;
        
        if ( pc != -1 && addr >= -2048 && addr <= 2047 ) {
          nowline.set_flag('b', 0 );
          nowline.set_flag('p', 1 );
          nowline.mObjCode += addr;
        } else {
          nowline.set_flag('b', 1 );
          nowline.set_flag('p', 0 );
          
          addr = opd.locc - rb;
          nowline.mObjCode += addr;
        }
        
        break;
        ////////////////////////////////////  
      case 4:
        nowline.mCodeSize = 4;
        nowline.set_flag('e', 1 );
        
        for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
          if ( t[k].type == INS ) break;
        } 
        
        idef = InsT.get(t[k].str);
        nowline.set_op(idef.opcode);
        
        if ( t[k+1].type == DELM ) {
          if ( lowcmp( t[k+1].str , "#" ) == 0 ) {
            nowline.set_flag('n', 0 );
            nowline.set_flag('i', 1 );
            if ( t[k+2].type == INT ) {
              sscanf(t[k+2].str.c_str(), "%d", &addr) ;
              nowline.mObjCode += addr;
              continue;
            }
          } else if ( lowcmp( t[k+1].str , "@" ) == 0 ) {
            nowline.set_flag('n', 1 );
            nowline.set_flag('i', 0 );
          } 
          k++;
        } else {
          nowline.set_flag('n', 1 );
          nowline.set_flag('i', 1 );
        }
        
        if ( lowcmp(t[k+3].str, "x") == 0 ) {
          nowline.set_flag('x', 1 );
        }
        
        if ( idef.opd_num == 0 ) continue;
        
        opd = SymbolT.get( t[k+1].str ) ;
        //output << "[%" << t[k].str<<" , "<< t[k+1].str << "]" << endl ;
        
        addr = opd.locc ;
        nowline.mObjCode += addr;
        
        break;
        ////////////////////////////////////  
      case 5:
        
        for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
          if ( t[k].type == CHAR ) break;
        } 
        
        if ( t[k].type == CHAR ) {
          int sz;
          for ( sz=0 ; sz<t[k].str.size() ; sz++ ) {
            nowline.mObjCode = nowline.mObjCode << 2 ;
            nowline.mObjCode += t[k].str[sz];
          }
        } else {
        
          for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
            if ( t[k].type == INT ) break;
          } 
          
          if ( t[k].type == INT ) {
            t[k].str = "0x" + t[k].str ;
            sscanf( t[k].str.c_str(), "%x", &nowline.mObjCode);
          }
          
        }
        
        break;
        ///////////////////////////
      case 6:
        
        for ( k = 0 ; k <= LLTable[i].mtop ; k++ ) {
          if ( t[k].type == INT ) break;
        } 
        
        if ( t[k].type == INT ) {
          t[k].str = "0x" + t[k].str ;
          sscanf( t[k].str.c_str(), "%x", &nowline.mObjCode);
        }
        
        break;
    }
    
  } // for i
  
} // pass2()


int main(int argc, const char * argv[])
{
  RegLT['A'] = 0; RegLT['a'] = 0;
  RegLT['X'] = 1; RegLT['x'] = 1;
  RegLT['L'] = 2; RegLT['l'] = 2;
  RegLT['B'] = 3; RegLT['b'] = 3;
  RegLT['S'] = 4; RegLT['s'] = 4;
  RegLT['T'] = 5; RegLT['t'] = 5;
  RegLT['F'] = 6; RegLT['f'] = 6;
  
  
  string input_filename, output_filename;
  cout << "Input File: " ;
  cin >> input_filename;
  cout << "Output File: " ;
  cin >> output_filename;
  input.open(input_filename.c_str(), fstream::in);
  output.open(output_filename.c_str(),fstream::out);
  
  
  
  Lex();
  pass1() ;
  pass2() ;
  
  
  
  int i ;
  output << "Line\tLoc\tSource\t\t\t\t\t\tObj Code" << endl;
  for ( i = 0 ; i <= nowll ; i++ ) {
    //line number
    output << (i*1)*5 << "\t" ;
    
    //comment or PSE
    if ( LLTable[i].mType == -1 || (LLTable[i].mType == 0 && LLTable[i].mloc == -1) ) {
      output << "\t" << LLTable[i].mSrc << endl ;
      continue;
    }
    
    //location
    output << std::hex << setfill('0') << setw(4) << std::uppercase << LLTable[i].mloc << std::dec << "\t" ;
    
    //source
    output << LLTable[i].mSrc << "\t\t\t\t\t\t" ;
    
    if ( strstr(lowstr(LLTable[i].mSrc).c_str(),"resb") != NULL ) {
      output << endl ; continue ;
    }
    if ( strstr(lowstr(LLTable[i].mSrc).c_str(),"resw") != NULL ) {
      output << endl ; continue ;
    }
    
    //obj code
    output << std::hex << setfill('0') << setw(LLTable[i].mCodeSize*2) << std::uppercase << LLTable[i].mObjCode << std::dec ;
    
    output << endl;
  }
  return 0 ;
}
