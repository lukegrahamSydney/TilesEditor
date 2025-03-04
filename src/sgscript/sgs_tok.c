
#include <math.h>

#include "sgs_int.h"


int sgsT_IsKeyword( sgs_TokenList tok, const char* text )
{
	return *tok == SGS_ST_KEYWORD && tok[ 1 ] == strlen( text ) &&
		memcmp( (const char*) tok + 2, text, tok[ 1 ] ) == 0;
}

int sgsT_IsIdent( sgs_TokenList tok, const char* text )
{
	return *tok == SGS_ST_IDENT && tok[ 1 ] == strlen( text ) &&
		memcmp( (const char*) tok + 2, text, tok[ 1 ] ) == 0;
}


static SGS_INLINE int detectline( const char* code, int32_t at )
{
	return code[ at ] == '\r' || ( code[ at ] == '\n' && ( at == 0 || code[ at - 1 ] != '\r' ) );
}

static void skipcomment( SGS_CTX, sgs_LineNum* line, const char* code, int32_t* at, int32_t length )
{
	int32_t i = *at + 1;
	if( code[ i ] == '/' )
	{
		i++;
		while( i < length && code[ i ] != '\n' && code[ i ] != '\r' )
			i++;
		if( i + 1 < length && code[ i ] == '\r' && code[ i + 1 ] == '\n' )
			i++;
		(*line)++;
		*at = i;
	}
	else
	{
		sgs_LineNum init = *line;
		i++;
		while( i < length )
		{
			if( detectline( code, i ) )
				(*line)++;
			if( code[ i ] == '/' && i > 0 && code[ i - 1 ] == '*' )
				break;
			else
				i++;
		}
		if( i == length )
		{
			sgs_Msg( C, SGS_ERROR, "[line %d] Comment has no end", init );
			*at = i - 1;
		}
		else
			*at = i;
	}
}

static int32_t string_inplace_fix( char* str, int32_t len )
{
	char *ipos = str, *opos = str, *iend = str + len;
	while( ipos < iend )
	{
		if( *ipos == '\\' )
		{
			ipos++;
			/* assumption that there's always a character after '\' */
			if( *ipos >= '0' && *ipos <= '7' )
			{
				int oct = *ipos++ - '0';
				if( ipos < iend && *ipos >= '0' && *ipos <= '7' ){ oct *= 8; oct += *ipos++ - '0'; }
				if( ipos < iend && *ipos >= '0' && *ipos <= '7' ){ oct *= 8; oct += *ipos++ - '0'; }
				ipos--;
				if( oct > 0xffff ) *opos++ = (char)( oct >> 8 );
				if( oct > 0xff ) *opos++ = (char)( oct >> 4 );
				*opos = (char) oct;
			}
			else
			{
				switch( *ipos )
				{
				case 'a': *opos = '\a'; break;
				case 'b': *opos = '\b'; break;
				case 'f': *opos = '\f'; break;
				case 'n': *opos = '\n'; break;
				case 'r': *opos = '\r'; break;
				case 't': *opos = '\t'; break;
				case 'v': *opos = '\v'; break;
				case 'x':
					if( ipos + 2 < iend && sgs_hexchar( ipos[1] ) && sgs_hexchar( ipos[2] ) )
					{
						*opos = (char)( ( sgs_gethex( ipos[1] ) << 4 ) | sgs_gethex( ipos[2] ) );
						ipos += 2;
						break;
					}
				/* ', ", \ too: */
				default: *opos = *ipos; break;
				}
			}
		}
		else
			*opos = *ipos;
		ipos++;
		opos++;
	}
	/* WP: returned string can only be shorter */
	return (int32_t) ( opos - str );
}


static int ident_equal( const char* ptr, int size, const char* what, int wlen )
{
	return size == wlen && memcmp( ptr, what, (size_t) size ) == 0;
}
static void readident( SGS_CTX, sgs_MemBuf* out, const char* code, int32_t* at, int32_t length, char xsym )
{
	int32_t sz = 0;
	int32_t i = *at;
	int32_t pos_rev = (int32_t) out->size;
	sgs_membuf_appchr( out, C, SGS_ST_IDENT );
	sgs_membuf_appchr( out, C, 0 );
	while( i < length && ( sgs_isalnum( code[ i ] ) || code[ i ] == '_' || code[ i ] == xsym ) )
	{
		sz++;
		if( sz < 256 )
			sgs_membuf_appchr( out, C, code[ i ] );
		else if( sz == 256 )
		{
			C->state |= SGS_HAS_ERRORS;
			sgs_Msg( C, SGS_ERROR, "[line %d] identifier too long", *at );
		}
		i++;
	}
	if( sz >= 255 ) sz = 255;
	out->ptr[ pos_rev + 1 ] = (char) sz;
	if( ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("var") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("global") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("thread") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("subthread") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("sync") ) ||
		//ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("race") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("null") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("true") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("false") ) ||
		ident_equal(out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("switch")) ||
		ident_equal(out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("case")) ||
		ident_equal(out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("default")) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("if") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("else") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("do") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("while") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("for") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("foreach") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("break") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("continue") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("function") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("use") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("return") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("this") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("new") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("defer") ) ||
		ident_equal( out->ptr + pos_rev + 2, sz, SGS_STRLITBUF("decltree") ) )
	{
		out->ptr[ pos_rev ] = SGS_ST_KEYWORD;
	}
	*at = --i;
} 
   
static void readstring( SGS_CTX, sgs_MemBuf* out, sgs_LineNum* line, const char* code, int32_t* at, int32_t length )
{
	int32_t begln = *line;
	int32_t i = *at + 1;
	char endchr = code[ *at ];
	int escaped = 0;
	
	if( i + 1 < length && code[i] == endchr && code[i+1] == endchr )
	{
		/* string without escape code parsing */
		int32_t beg = i + 2;
		i = beg + 2;
		while( i < length )
		{
			char c = code[ i ];
			if( detectline( code, i - 2 ) )
				(*line)++;
			if( c == endchr && code[i-1] == endchr && code[i-2] == endchr )
			{
				int32_t size = i - beg - 2;
				sgs_membuf_appchr( out, C, SGS_ST_STRING );
				sgs_membuf_appbuf( out, C, (char*) &size, 4 );
				sgs_membuf_appbuf( out, C, code + beg, (size_t) size );
				*at = i;
				return;
			}
			i++;
		}
	}
	
	while( i < length )
	{
		char c = code[ i ];
		if( detectline( code, i ) )
			(*line)++;
		if( c == '\\' )
			escaped = !escaped;
		else if( c == endchr && !escaped )
		{
			int32_t size = i - *at - 1, newsize;
			size_t numpos = out->size + 1;
			sgs_membuf_appchr( out, C, SGS_ST_STRING );
			sgs_membuf_appbuf( out, C, (char*) &size, 4 );
			sgs_membuf_appbuf( out, C, code + *at + 1, (size_t) size );
			*at += size + 1;
			newsize = string_inplace_fix( out->ptr + numpos + 4, size );
			memcpy( out->ptr + numpos, &newsize, sizeof(newsize) );
			/* WP: size-newsize always non-negative */
			out->size -= (size_t)( size - newsize );
			return;
		}
		else
			escaped = 0;
		i++;
	}
	
	C->state |= SGS_MUST_STOP;
	sgs_Msg( C, SGS_ERROR, "[line %d] end of string not found", begln );
}

static const char* sgs_opchars = "=<>+-*/%?!~&|^.$@";
static const char* sgs_operators = "<=>;===;!==;==;!=;<=;>=;+=;-=;*=;/=;%=;&=;|=;^=;<<=;>>=;$=;..=;"
	"<<;>>;&&=;||=;?""?=;&&;||;?""?;..;<;>;=;++;--;+;-;*;/;%;&;|;^;.;$;!;~;@;?;?."; /* trigraphs detected */
static const sgs_TokenType sgs_optable[] =
{
	SGS_ST_OP_RWCMP, SGS_ST_OP_SEQ, SGS_ST_OP_SNEQ, SGS_ST_OP_EQ, SGS_ST_OP_NEQ, SGS_ST_OP_LEQ, SGS_ST_OP_GEQ,
	SGS_ST_OP_ADDEQ, SGS_ST_OP_SUBEQ, SGS_ST_OP_MULEQ, SGS_ST_OP_DIVEQ, SGS_ST_OP_MODEQ,
	SGS_ST_OP_ANDEQ, SGS_ST_OP_OREQ, SGS_ST_OP_XOREQ, SGS_ST_OP_LSHEQ, SGS_ST_OP_RSHEQ, SGS_ST_OP_CATEQ, SGS_ST_OP_CATEQ,
	SGS_ST_OP_LSH, SGS_ST_OP_RSH, SGS_ST_OP_BLAEQ, SGS_ST_OP_BLOEQ, SGS_ST_OP_NLOEQ, SGS_ST_OP_BLAND,
	SGS_ST_OP_BLOR, SGS_ST_OP_NLOR, SGS_ST_OP_CAT, SGS_ST_OP_LESS, SGS_ST_OP_GRTR, SGS_ST_OP_SET, SGS_ST_OP_INC, SGS_ST_OP_DEC,
	SGS_ST_OP_ADD, SGS_ST_OP_SUB, SGS_ST_OP_MUL, SGS_ST_OP_DIV, SGS_ST_OP_MOD, SGS_ST_OP_AND,
	SGS_ST_OP_OR, SGS_ST_OP_XOR, SGS_ST_OP_MMBR, SGS_ST_OP_CAT, SGS_ST_OP_NOT, SGS_ST_OP_INV, SGS_ST_OP_ERSUP, SGS_ST_OP_QMARK,SGS_ST_OP_QMARK_CALL
};
static const char sgs_opsep = ';';

static int op_oneof( const char* str, const char* test, char sep, int* outlen )
{
	const char* pstr = str;
	int passed = 0, equal = 0, which = 0, len = 0;
	
	do
	{
		if( *test == sep || *test == 0 )
		{
			if( passed == equal )
			{
				*outlen = len;
				return which;
			}
			if( *test == 0 )
				return -1;
			passed = 0;
			equal = 0;
			len = 0;
			pstr = str;
			which++;
		}
		else
		{
			len++;
			passed++;
			if( *pstr == *test )
				equal++;
			pstr += !!*pstr;
		}
	}
	while( *test++ );
	
	return -1;
}

static void readop( SGS_CTX, sgs_MemBuf* out, sgs_LineNum line, const char* code, int32_t* at, int32_t length )
{
	char opstr[ 4 ];
	int32_t opsize = 0, ropsize = 0, i = *at, whichop, len = -1;
	
	memset( opstr, 0, 4 );
	
	/* read in the operator */
	while( i < length && sgs_isoneof( code[ i ], sgs_opchars ) )
	{
		if( opsize < 3 )
			opstr[ opsize++ ] = code[ i ];
		ropsize++;
		i++;
	}
	
	/* test for various errors */
	if( ropsize > 3 ){ *at += ropsize; goto op_read_error; }
	whichop = op_oneof( opstr, sgs_operators, sgs_opsep, &len );
	*at += len - 1;
	if( whichop < 0 ) goto op_read_error;
	
	sgs_membuf_appchr( out, C, (char) sgs_optable[ whichop ] );
	return;
	
op_read_error:
	C->state |= SGS_HAS_ERRORS;
	sgs_Msg( C, SGS_ERROR, "[line %d] invalid operator found: \"%s%s\", size=%d", line, opstr, ropsize > 3 ? "..." : "", ropsize );
}
 
sgs_TokenList sgsT_Gen( SGS_CTX, const char* code, size_t length )
{
	int32_t i, ilen = (int32_t) length; /* WP: code limit */
	sgs_LineNum line = 1;
	sgs_ParserConfig pcfg = C->shared->parser_cfg;
	sgs_MemBuf s = sgs_membuf_create();
	sgs_membuf_reserve( &s, C, SGS_TOKENLIST_PREALLOC );
	
	for( i = 0; i < ilen; ++i )
	{
		sgs_LineNum tokline = line;
		char fc = code[ i ];
		size_t isz = s.size;
		
		/* whitespace */
		if( detectline( code, i ) )
			line++;
		if( sgs_isoneof( fc, " \n\r\t" ) )
			continue;
		
		/* comment */
		if( fc == '/' &&
			i + 1 < ilen &&
			( code[ i + 1 ] == '/' ||
				code[ i + 1 ] == '*' ) )   skipcomment( C, &line, code, &i, ilen );
		
		/* special symbol */
		else if( sgs_isoneof( fc, "()[]{},;:#\\" ) ) sgs_membuf_appchr( &s, C, fc );
		
		/* identifier */
		else if( fc == '_' || sgs_isalpha( fc ) || ( fc == '$' && pcfg.ident_dollar_sign ) )
			readident( C, &s, code, &i, ilen, pcfg.ident_dollar_sign ? '$' : '_' );
		
		/* number */
		else if( sgs_isdigit( fc ) )
		{
			sgs_Int vi = 0;
			sgs_Real vr = 0;
			const char* pos = code + i;
			int res = sgs_util_strtonum( &pos, code + length, &vi, &vr );
			if( res == 0 )
			{
				C->state |= SGS_HAS_ERRORS;
				sgs_Msg( C, SGS_ERROR, "[line %d] failed to parse numeric constant", line );
			}
			else
			{
				if( pos < code + length && ( *pos == 'f' || *pos == '.' ) )
				{
					pos++;
					if( res == 1 )
					{
						res = 2;
						vr = (sgs_Real) vi;
					}
				}
				else if( pos < code + length && res == 1 && *pos == 'p' )
				{
					pos++;
					res = 3;
				}
				if( res == 1 )
				{
					sgs_membuf_appchr( &s, C, SGS_ST_NUMINT );
					sgs_membuf_appbuf( &s, C, &vi, sizeof( vi ) );
				}
				else if( res == 2 )
				{
					sgs_membuf_appchr( &s, C, SGS_ST_NUMREAL );
					sgs_membuf_appbuf( &s, C, &vr, sizeof( vr ) );
				}
				/* not an original return value */
				else if( res == 3 )
				{
					sgs_membuf_appchr( &s, C, SGS_ST_NUMPTR );
					sgs_membuf_appbuf( &s, C, &vi, sizeof( vi ) );
				}
			}
			/* WP: code limit */
			i = (int32_t) ( pos - code );
			i--;
		}
		
		
		//single quote thing
		else if (fc == '\'')
		{
			char buffer[30];
			int32_t buffLen = 0;

			sgs_LineNum begln = line;
			int noEndQuoteError = 1;
			int escaped = 0;
			for (int ii = i + 1; ii < ilen; ++ii)
			{
				if (detectline(code, ii))
					++line;
				
				if (code[ii] == '\'' && !escaped) {

					int32_t newsize = string_inplace_fix(buffer, buffLen);
					sgs_Int value = 0;
					for (int32_t iii = 0; iii < newsize; ++iii)
					{
						value <<= 8;
						value |= buffer[iii];
					}
					sgs_membuf_appchr(&s, C, SGS_ST_NUMINT);
					sgs_membuf_appbuf(&s, C, &value, sizeof(value));

					i = ii;
					noEndQuoteError = 0;
					break;
				}

				if (buffLen < sizeof(buffer)) {
					buffer[buffLen++] = code[ii];

					if (code[ii] == '\\')
						escaped = 1;
					else escaped = 0;
				}
				else {
					C->state |= SGS_MUST_STOP;
					sgs_Msg(C, SGS_ERROR, "[line %d] characters in single quote exceeded buffer size (30 characters)", begln);
					noEndQuoteError = 0;
					break;
				}
			}

			if(noEndQuoteError)
			{
				C->state |= SGS_MUST_STOP;
				sgs_Msg(C, SGS_ERROR, "[line %d] end of single quote not found", begln);
			}
		}
		/* string */
		else if( fc == '\"' )	      readstring( C, &s, &line, code, &i, ilen );
		
		/* operator */
		else if( sgs_isoneof( fc, sgs_opchars ) ) readop( C, &s, line, code, &i, ilen );
		
		/* - unexpected symbol */
		else
		{
			C->state |= SGS_HAS_ERRORS;
			sgs_Msg( C, SGS_ERROR, "[line %d], unexpected symbol: %c", line, fc );
		}
		
		if( s.size != isz ) /* write a line only if successfully wrote something (a token) */
			sgs_membuf_appbuf( &s, C, &tokline, sizeof( tokline ) );
		
		if( C->state & SGS_MUST_STOP )
			break;
	}
	
	sgs_membuf_appchr( &s, C, SGS_ST_NULL );
	return (sgs_TokenList) s.ptr;
}

void sgsT_Free( SGS_CTX, sgs_TokenList tlist )
{
	sgs_MemBuf s = sgs_membuf_partial( (char*) tlist, 0 );
	sgs_membuf_destroy( &s, C );
}

sgs_TokenList sgsT_Next( sgs_TokenList tok )
{
	sgs_BreakIf( !tok );
	sgs_BreakIf( !*tok );
	
	switch( *tok )
	{
	case SGS_ST_IDENT:
	case SGS_ST_KEYWORD:
		return tok + tok[ 1 ] + 2 + sizeof( sgs_LineNum );
	case SGS_ST_NUMREAL:
	case SGS_ST_NUMINT:
	case SGS_ST_NUMPTR:
		return tok + 9 + sizeof( sgs_LineNum );
	case SGS_ST_STRING:
		{
			int32_t len;
			SGS_ST_READINT( len, tok + 1 );
			return tok + 5 + sizeof( sgs_LineNum ) + len;
		}
	default:
		return tok + 1 + sizeof( sgs_LineNum );
	}
}

sgs_LineNum sgsT_LineNum( sgs_TokenList tok )
{
	if( !tok || !*tok )
		return -1;
	tok = sgsT_Next( tok );
	{
		sgs_LineNum ln;
		SGS_ST_READLN( ln, tok - 2 );
		return ln;
	}
}


size_t sgsT_ListSize( sgs_TokenList tlist )
{
	size_t i = 0;
	while( *tlist )
	{
		tlist = sgsT_Next( tlist );
		i++;
	}
	return i;
}

size_t sgsT_ListMemSize( sgs_TokenList tlist )
{
	sgs_TokenList last = tlist;
	while( *last )
		last = sgsT_Next( last );
	return (size_t) ( last - tlist + 1 );
}


static void tp_token( SGS_CTX, sgs_MemBuf* out, sgs_TokenList t )
{
	switch( *t )
	{
	case SGS_ST_RBRKL:
	case SGS_ST_RBRKR:
	case SGS_ST_SBRKL:
	case SGS_ST_SBRKR:
	case SGS_ST_CBRKL:
	case SGS_ST_CBRKR:
	case SGS_ST_ARGSEP:
	case SGS_ST_STSEP:
	case SGS_ST_PICKSEP:
	case SGS_ST_HASH:
	case SGS_ST_BACKSLASH:
		sgs_membuf_appchr( out, C, (char) *t );
		break;
	case SGS_ST_IDENT:
	case SGS_ST_KEYWORD:
		sgs_membuf_appbuf( out, C, t + 2, t[1] );
		break;
	case SGS_ST_NUMREAL:
		{
			sgs_Real val;
			char tmp[ 64 ];
			SGS_AS_REAL( val, t+1 );
			snprintf( tmp, 63, "%g", val );
			sgs_membuf_appbuf( out, C, tmp, strlen( tmp ) );
		}
		break;
	case SGS_ST_NUMINT:
		{
			sgs_Int val;
			char tmp[ 24 ];
			SGS_AS_INTEGER( val, t+1 );
			sprintf( tmp, "%" PRId64, val );
			sgs_membuf_appbuf( out, C, tmp, strlen( tmp ) );
		}
		break;
	case SGS_ST_NUMPTR:
		{
			sgs_Int val;
			char tmp[ 24 ];
			SGS_AS_INTEGER( val, t+1 );
			sprintf( tmp, "%" PRIx64 "p", val );
			sgs_membuf_appbuf( out, C, tmp, strlen( tmp ) );
		}
		break;
	case SGS_ST_STRING:
		{
			int32_t i, size;
			SGS_ST_READINT( size, t + 1 );
			sgs_TokenList buf = t + 5;
			for( i = 0; i < size; ++i )
			{
				if( sgs_isgraph( buf[ i ] ) || buf[ i ] == ' ' )
					sgs_membuf_appchr( out, C, (char) buf[ i ] );
				else
				{
					static const char* hexdigs = "0123456789ABCDEF";
					char tmp[ 4 ] = { '\\', 'x', 0, 0 };
					tmp[2] = hexdigs[ (buf[i] & 0xf0) >> 4 ];
					tmp[3] = hexdigs[ buf[i] & 0xf ];
					sgs_membuf_appbuf( out, C, tmp, 4 );
				}
			}
		}
		break;
#define OPR( op ) sgs_membuf_appbuf( out, C, SGS_STRLITBUF(op) )
	case SGS_ST_OP_RWCMP: OPR( "<=>" ); break;
	case SGS_ST_OP_SEQ: OPR( "===" ); break;
	case SGS_ST_OP_SNEQ: OPR( "!==" ); break;
	case SGS_ST_OP_EQ: OPR( "==" ); break;
	case SGS_ST_OP_NEQ: OPR( "!=" ); break;
	case SGS_ST_OP_LEQ: OPR( "<=" ); break;
	case SGS_ST_OP_GEQ: OPR( ">=" ); break;
	case SGS_ST_OP_ADDEQ: OPR( "+=" ); break;
	case SGS_ST_OP_SUBEQ: OPR( "-=" ); break;
	case SGS_ST_OP_MULEQ: OPR( "*=" ); break;
	case SGS_ST_OP_DIVEQ: OPR( "/=" ); break;
	case SGS_ST_OP_MODEQ: OPR( "%=" ); break;
	case SGS_ST_OP_ANDEQ: OPR( "&=" ); break;
	case SGS_ST_OP_OREQ: OPR( "|=" ); break;
	case SGS_ST_OP_XOREQ: OPR( "^=" ); break;
	case SGS_ST_OP_LSHEQ: OPR( "<<=" ); break;
	case SGS_ST_OP_RSHEQ: OPR( ">>=" ); break;
	case SGS_ST_OP_BLAEQ: OPR( "&&=" ); break;
	case SGS_ST_OP_BLOEQ: OPR( "||=" ); break;
	case SGS_ST_OP_CATEQ: OPR( "$=" ); break;
	case SGS_ST_OP_BLAND: OPR( "&&" ); break;
	case SGS_ST_OP_BLOR: OPR( "||" ); break;
	case SGS_ST_OP_LESS: OPR( "<" ); break;
	case SGS_ST_OP_GRTR: OPR( ">" ); break;
	case SGS_ST_OP_SET: OPR( "=" ); break;
	case SGS_ST_OP_ADD: OPR( "+" ); break;
	case SGS_ST_OP_SUB: OPR( "-" ); break;
	case SGS_ST_OP_MUL: OPR( "*" ); break;
	case SGS_ST_OP_DIV: OPR( "/" ); break;
	case SGS_ST_OP_MOD: OPR( "%" ); break;
	case SGS_ST_OP_AND: OPR( "&" ); break;
	case SGS_ST_OP_OR: OPR( "|" ); break;
	case SGS_ST_OP_XOR: OPR( "^" ); break;
	case SGS_ST_OP_LSH: OPR( "<<" ); break;
	case SGS_ST_OP_RSH: OPR( ">>" ); break;
	case SGS_ST_OP_MMBR: OPR( "." ); break;
	case SGS_ST_OP_CAT: OPR( "$" ); break;
	case SGS_ST_OP_NOT: OPR( "!" ); break;
	case SGS_ST_OP_INV: OPR( "~" ); break;
	case SGS_ST_OP_INC: OPR( "++" ); break;
	case SGS_ST_OP_DEC: OPR( "--" ); break;
	case SGS_ST_OP_QMARK: OPR( "?" ); break;
	case SGS_ST_OP_QMARK_CALL: OPR("?."); break;
#undef OPR
	default:
		sgs_membuf_appbuf( out, C, "<error>", 7 );
		break;
	}
}

static int tp_tt2i( sgs_TokenType t )
{
	/* 0 ident | 1 const | 2 punct | 3 op */
	if( SGS_ST_ISOP( t ) ) return 3;
	if( t == SGS_ST_IDENT || t == SGS_ST_KEYWORD ) return 0;
	if( t == SGS_ST_NUMREAL || t == SGS_ST_NUMINT || t == SGS_ST_NUMPTR || t == SGS_ST_STRING ) return 1;
	return 2;
}

/*
	kerning table:
	  I C P O
	I 1 1 0 +
	C 1 1 0 +
	P 0 0 0 +
	O + + + 1
*/
static void tp_kerning( SGS_CTX, sgs_MemBuf* out, sgs_TokenList t1, sgs_TokenList t2, int xs )
{
	static const int32_t mask = 0x8033;
	static const int32_t maskg = 0xf8bb;
	int32_t m;
	int ty1, ty2;
	
	if( !t1 || !t2 )
		return;
	
	m = xs ? maskg : mask;
	ty1 = tp_tt2i( *t1 );
	ty2 = tp_tt2i( *t2 );
	
	if( m & (1 << (ty1 + ty2 * 4)) )
		sgs_membuf_appchr( out, C, ' ' );
}

void sgsT_TokenString( SGS_CTX, sgs_MemBuf* out, sgs_TokenList tlist, sgs_TokenList tend, int xs )
{
	while( tlist < tend && *tlist )
	{
		sgs_TokenList t = tlist;
		tlist = sgsT_Next( t );
		
		tp_token( C, out, t );
		tp_kerning( C, out, t, tlist == tend ? NULL : tlist, xs );
	}
}


void sgsT_DumpToken( SGS_CTX, sgs_TokenList tok )
{
	switch( *tok )
	{
	case SGS_ST_RBRKL:
	case SGS_ST_RBRKR:
	case SGS_ST_SBRKL:
	case SGS_ST_SBRKR:
	case SGS_ST_CBRKL:
	case SGS_ST_CBRKR:
	case SGS_ST_ARGSEP:
	case SGS_ST_STSEP:
	case SGS_ST_PICKSEP:
	case SGS_ST_HASH:
	case SGS_ST_BACKSLASH:
		sgs_ErrWritef( C, "%c", *tok );
		break;
	case SGS_ST_IDENT:
		sgs_ErrWritef( C, "id(" );
		sgs_WriteSafe( (sgs_ErrorOutputFunc) sgs_ErrWritef, C, (const char*) tok + 2, tok[ 1 ] );
		sgs_ErrWritef( C, ")" );
		break;
	case SGS_ST_KEYWORD:
		sgs_ErrWritef( C, "[" );
		sgs_WriteSafe( (sgs_ErrorOutputFunc) sgs_ErrWritef, C, (const char*) tok + 2, tok[ 1 ] );
		sgs_ErrWritef( C, "]" );
		break;
	case SGS_ST_NUMREAL:
		{
			sgs_Real val;
			SGS_AS_REAL( val, tok + 1 );
			sgs_ErrWritef( C, "real(%f)", val );
		}
		break;
	case SGS_ST_NUMINT:
		{
			sgs_Int val;
			SGS_AS_INTEGER( val, tok + 1 );
			sgs_ErrWritef( C, "int(%" PRId64 ")", val );
		}
		break;
	case SGS_ST_NUMPTR:
		{
			sgs_Int val;
			SGS_AS_INTEGER( val, tok + 1 );
			sgs_ErrWritef( C, "ptr(%" PRIx64 ")", val );
		}
		break;
	case SGS_ST_STRING:
		{
			int32_t len;
			SGS_ST_READINT( len, tok + 1 );
			sgs_ErrWritef( C, "str(" );
			sgs_WriteSafe( (sgs_ErrorOutputFunc) sgs_ErrWritef, C, (const char*) tok + 5, (size_t) len );
			sgs_ErrWritef( C, ")" );
		}
		break;
#define OPR( op ) sgs_ErrWritef( C, "%s", op );
	case SGS_ST_OP_RWCMP: OPR( "<=>" ); break;
	case SGS_ST_OP_SEQ: OPR( "===" ); break;
	case SGS_ST_OP_SNEQ: OPR( "!==" ); break;
	case SGS_ST_OP_EQ: OPR( "==" ); break;
	case SGS_ST_OP_NEQ: OPR( "!=" ); break;
	case SGS_ST_OP_LEQ: OPR( "<=" ); break;
	case SGS_ST_OP_GEQ: OPR( ">=" ); break;
	case SGS_ST_OP_ADDEQ: OPR( "+=" ); break;
	case SGS_ST_OP_SUBEQ: OPR( "-=" ); break;
	case SGS_ST_OP_MULEQ: OPR( "*=" ); break;
	case SGS_ST_OP_DIVEQ: OPR( "/=" ); break;
	case SGS_ST_OP_MODEQ: OPR( "%=" ); break;
	case SGS_ST_OP_ANDEQ: OPR( "&=" ); break;
	case SGS_ST_OP_OREQ: OPR( "|=" ); break;
	case SGS_ST_OP_XOREQ: OPR( "^=" ); break;
	case SGS_ST_OP_LSHEQ: OPR( "<<=" ); break;
	case SGS_ST_OP_RSHEQ: OPR( ">>=" ); break;
	case SGS_ST_OP_BLAEQ: OPR( "&&=" ); break;
	case SGS_ST_OP_BLOEQ: OPR( "||=" ); break;
	case SGS_ST_OP_CATEQ: OPR( "$=" ); break;
	case SGS_ST_OP_BLAND: OPR( "&&" ); break;
	case SGS_ST_OP_BLOR: OPR( "||" ); break;
	case SGS_ST_OP_LESS: OPR( "<" ); break;
	case SGS_ST_OP_GRTR: OPR( ">" ); break;
	case SGS_ST_OP_SET: OPR( "=" ); break;
	case SGS_ST_OP_ADD: OPR( "+" ); break;
	case SGS_ST_OP_SUB: OPR( "-" ); break;
	case SGS_ST_OP_MUL: OPR( "*" ); break;
	case SGS_ST_OP_DIV: OPR( "/" ); break;
	case SGS_ST_OP_MOD: OPR( "%" ); break;
	case SGS_ST_OP_AND: OPR( "&" ); break;
	case SGS_ST_OP_OR: OPR( "|" ); break;
	case SGS_ST_OP_XOR: OPR( "^" ); break;
	case SGS_ST_OP_LSH: OPR( "<<" ); break;
	case SGS_ST_OP_RSH: OPR( ">>" ); break;
	case SGS_ST_OP_MMBR: OPR( "." ); break;
	case SGS_ST_OP_ERSUP: OPR( "@" ); break;
	case SGS_ST_OP_CAT: OPR( "$" ); break;
	case SGS_ST_OP_NOT: OPR( "!" ); break;
	case SGS_ST_OP_INV: OPR( "~" ); break;
	case SGS_ST_OP_INC: OPR( "++" ); break;
	case SGS_ST_OP_DEC: OPR( "--" ); break;
#undef OPR
	default:
		sgs_ErrWritef( C, "<invalid>" );
		break;
	}
	sgs_ErrWritef( C, "@%d", sgsT_LineNum( tok ) );
}
void sgsT_DumpList( SGS_CTX, sgs_TokenList tlist, sgs_TokenList tend )
{
	sgs_ErrWritef( C, "\n" );
	while( tlist != tend && *tlist != 0 )
	{
		sgs_ErrWritef( C, "   " );
		sgsT_DumpToken( C, tlist );
		tlist = sgsT_Next( tlist );
	}
	sgs_ErrWritef( C, "\n\n" );
}
