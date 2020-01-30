
// Specify version of library to use

(**)
{$IFDEF BUILDLITE}
    {$DEFINE MGLLITE}
{$ELSE}
    {$DEFINE MGLFIX3D}
{$ENDIF}
(**)

// If code above omited then use following defaults

{$IFNDEF MGLFIX3D}
    {$IFNDEF MGLLITE}
        {$DEFINE MGLFIX3D}
    {$ENDIF}
{$ENDIF}

{$IFDEF MGLFIX3D}
    {$DEFINE MGL3D}
{$ENDIF}
