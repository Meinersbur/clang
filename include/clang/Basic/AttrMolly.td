
def MollyField : InheritableAttr {
  let Spellings = [GNU<"molly_field">, CXX11<"molly","field">];
}

def MollyDimensions : InheritableAttr {
  let Spellings = [GNU<"molly_dimensions">, CXX11<"molly","dimensions">];
  let Args = [IntArgument<"Dimensions">];
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyFieldLengths : InheritableAttr {
  let Spellings = [GNU<"molly_lengths">, CXX11<"molly","lengths">]; // [CXX11<"molly","lengths">]
  //  let Subjects = [CXXRecord];
  let Args = [VariadicExprArgument<"lengths">];
  let LateParsed = 1;
  let TemplateDependent = 1;
}


def MollyLengthFunc : InheritableAttr {
  let Spellings = [GNU<"molly_lengthfunc">];
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyGetterFunc : InheritableAttr {
  let Spellings = [GNU<"molly_getterfunc">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollySetterFunc : InheritableAttr {
  let Spellings = [GNU<"molly_setterfunc">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyRefFunc : InheritableAttr {
  let Spellings = [GNU<"molly_reffunc">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyFieldmember : InheritableAttr {
  let Spellings = [GNU<"molly_fieldmember">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyIsLocalFunc : InheritableAttr {
  let Spellings = [GNU<"molly_islocalfunc">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyGetRankOfFunc : InheritableAttr {
  let Spellings = [GNU<"molly_getrankoffunc">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyInline : InheritableAttr {
  let Spellings = [GNU<"molly_inline">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyGetBroadcast : InheritableAttr {
  let Spellings = [GNU<"molly_get_broadcast">, CXX11<"molly","get_broadcast">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollyGetMaster : InheritableAttr {
  let Spellings = [GNU<"molly_get_master">, CXX11<"molly","get_master">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollySetBroadcast : InheritableAttr {
  let Spellings = [GNU<"molly_set_broadcast">, CXX11<"molly","set_broadcast">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}

def MollySetMaster : InheritableAttr {
  let Spellings = [GNU<"molly_set_master">, CXX11<"molly","set_master">]; 
  let LateParsed = 1;
  let TemplateDependent = 1;
}
