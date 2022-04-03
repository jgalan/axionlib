#!/usr/bin/python3

import ROOT

ROOT.gSystem.Load("libRestFramework.so")
ROOT.gSystem.Load("libRestAxion.so")

mirror = ROOT.TRestAxionOpticsMirror("mirrors.rml", "default")
mirror.PrintMetadata()

ref1  = mirror.GetReflectivity( 0.25, 1 )
ref2  = mirror.GetReflectivity( 0.5, 3 )
ref3  = mirror.GetReflectivity( 0.75, 5 )
ref4  = mirror.GetReflectivity( 1., 7 )

ref1int = int(10000. * ref1 )
ref2int = int(10000. * ref2 )
ref3int = int(10000. * ref3 )
ref4int = int(10000. * ref4 )
print( "Reflectivity: " + str(ref1int), end ="" )
if( ref1int != 9721 ):
    print( "\nError! Reflectivity should be 0.9721!" )
    exit(1)
print (" [\033[92m OK \x1b[0m]")

print( "Reflectivity: " + str(ref2int), end ="" )
if( ref2int != 9563 ):
    print( "\nError! Reflectivity should be 0.9563!" )
    exit(1)
print (" [\033[92m OK \x1b[0m]")

print( "Reflectivity: " + str(ref3int), end ="" )
if( ref3int != 37 ):
    print( "\nError! Reflectivity should be 0.0037!" )
    exit(1)
print (" [\033[92m OK \x1b[0m]")

print( "Reflectivity: " + str(ref4int), end ="" )
if( ref4int != 2 ):
    print( "\nError! Reflectivity should be 0.0005!" )
    exit(1)
print (" [\033[92m OK \x1b[0m]")


print ("All tests passed!")

exit(0)
