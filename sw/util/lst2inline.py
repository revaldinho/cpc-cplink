#!/usr/bin/env python3
''' 
asm2inline.py

  Convert an ASM80 listing into BCPL inline statements, by default also including comments and code

USAGE

  python3 asm2inline.py -f|--filename <filename> [-g|--group int][-n|--nocomment] [-o|--nocode][-d|--decimal][-h|--hex][-p|--compact][-b|--codeblock]

MANDATORY SWITCHES

  -f, --filename  <filename>  filename of an ASM80 listing file

OPTIONAL SWITCHES

  -n, --nocomment             omits all comments from the listing
  -o, --nocode                omits all code and comments from the listing  
  -d, --decimal               use decimal for object bytes
  -h, --hex                   use hexadecimal for object bytes
  -g, --group  integer        digits group size when writing blocks with no asm code
  -p, --compact               minimizes spaces when including code and comments
  -b, --codeblock             separate asm/comments from block on INLINE statements
EXAMPLE

  ./asm2inline.py -f fifo.lst --nocomment

NOTES
'''
import sys, getopt, os.path

filename = ""
nocomment = False
nocode = False
format = "#x%02X"
groupsize = 8
compact = False
codeblock = False

def showUsageAndExit() :
    print ( __doc__)
    sys.exit(2)


def convert_file(file):
    address = list()
    object = list()
    asm = list ()
    comment = list ()

    for l in file:
        if len(l.strip())>0 and not l.startswith('#') :
            address.append(int(l[0:4],16))
            line_obj  = []
            label = ''
            for x in (l[5:18]).split():
                if  (x.lstrip()).startswith(';'):
                    break
                if not x.endswith(':') :
                    line_obj.append(int(x.strip(),16))
                else:
                    label = x
            object.append(line_obj)
            parts  = l[18:].split(';') + ['','']
            parts[0] = ' '.join( [label,parts[0]])
            if not nocode:
                asm.append(parts[0].strip())
                if not nocomment:
                    comment.append(parts[1].strip())    
                else:
                    comment.append('')

    print("  // Listing source: %s" % filename)

    codeformat = '%-8s' if not compact else '%s'
    objectformat = '%-18s' if not compact else '%s'
    leading = "%27s" if not compact and not codeblock else "%1s"
    
    if codeblock:                    
        outstr = []
        for (a,c) in zip(asm,comment):
            if len(c)>0 or len(a)>0:
                outstr.append( (leading + " // %s%s%s") %  ( '',' '.join([codeformat % b for b in a.split()]),'; ' if len(c)>0 else '', ' '.join(c.split())))
        print ('\n'.join(outstr))
    if nocode or codeblock:
        # No asm code at all - then write out groups of 8 bytes to minimise source code size
        bytes = []
        for bytelist in object:
            bytes.extend(bytelist)
        for i in range(0, len(bytes), groupsize):
            byteline = [ format % bytes[j] for j in range( i, min(i+groupsize,len(bytes)))]
            print ( "  INLINE %s" % ','.join(byteline))            
    else:
        for i in range (0, len(address) ):
            if len(object[i]) > 0 :
                outstr = ["  inline "+ objectformat % (",".join([format % b for b in object[i]]))]
                if (len(asm[i])>0 ):
                    outstr.append(" // %s " %  (' '.join([codeformat % b for b in asm[i].split()])))
                    if not nocomment and (len(comment[i])>0):
                        outstr.append("; %s" % comment[i].strip())
                print(''.join(outstr))
            else:
                if not nocomment:
                    leading = "%27s" if not compact else "%1s"
                    print( (leading+" // %s ; %s") % ( '',' '.join([codeformat % b for b in asm[i].split()]), ' '.join(comment[i].split())))
        

if __name__=="__main__":    

    try:
        opts, args = getopt.getopt( sys.argv[1:], "f:g:nohdxpb", ["filename=","group=","nocomment","nocode","help","decimal","hex","compact","codeblock"])
    except getopt.GetoptError:
        showUsageAndExit()
    for opt, arg in opts:
        if opt in ( "-f", "--filename" ) :
            filename = arg
        if opt in ( "-g", "--group" ) :
            groupsize = int(arg,0)
        elif opt in ( "-n", "--nocomment" ) :
            nocomment=True
        elif opt in ( "-o","--nocode" ) :            
            nocomment=True
            nocode=True
        elif opt in ( "-d","--decimal" ) :            
            format = "%d"
        elif opt in ( "-x","--hex" ) :            
            format = "#x%02X"
        elif opt in ( "-p","--compact" ) :            
            compact = True
        elif opt in ( "-b","--codeblock" ) :            
            codeblock = True
        elif opt in ( "-h","--help" ) :            
            showUsageAndExit()

    if filename=="" :
        showUsageAndExit()
    elif  not os.path.exists(filename):
        print("ERROR: file %s doesn't exist" % filename)
        sys.exit(2)

    file = open( filename, "r")
    convert_file(file)
    
    
