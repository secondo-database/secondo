operator setscalefactor alias SETSCALEFACTOR pattern op ( _, _ )
operator setregion2outstyle alias SETOUTSTYLE pattern op ( _)

operator isempty alias ISEMPTY pattern op( _ )
operator intersects alias INTERSECTS pattern _ infixop _
operator inside alias INSIDE pattern _ infixop _
operator adjacent alias ADJACENT pattern _ infixop _
operator overlaps alias OVERLAPS pattern _ infixop _
operator no_components alias NO_COMPONENTS pattern  op ( _ )
operator no_segments alias NO_SEGMENTS pattern  op ( _ )
operator bbox alias BBOX pattern  op ( _, _ )
operator translate alias TRANSLATE pattern  _ op [list]
operator scale alias SCALE pattern _ op [ _, _ ]
operator scale2 alias SCALE2 pattern _ op [ _, _ ]
operator components alias COMPONENTS pattern op(_)
operator getHoles alias GETHOLES pattern op(_)
operator region2region2 alias REGION2REGION2 pattern _ op
operator regiontoregion2 alias REGIONTOREGION2 pattern op ( _, _ )
operator rect2region2 alias RECT2REGION2 pattern _ op
operator recttoregion2 alias RECTTOREGION2 pattern op ( _, _ )
operator size alias SIZE pattern op ( _ )
operator area alias AREA pattern op ( _ )

