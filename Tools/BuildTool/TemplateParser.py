import re
import os

TokenDictionary = {
    "UNKNOWN_PLATFORM" : 0,
    "TestVal" : 10,
    "TestVal2" : 10,
    "TestVal3" : 0
}

def IdentifyParamTypeAndCleanup(Param : str, Index) -> tuple[str, str] | None:
    if (Param is None) or (len(Param) == 0):
        return (None, "")
    
    Index = GetNextValidCharacterIndex(Param, Index)

    Type = ""
    assert (Index < len(Param)), "Invalid Index retrieved"

    c = Param[Index]
    if c == '{' : 
        Type = "Dictionary"
    elif c == '[' or c == '(':
        Type = "List"
    elif ord(c) >= 48 and ord(c) <= 57:
        Type = "Number"
    else:
        Type = "String"

    return (Type, Index, Param)

def EvaluateBooleanExpression(Tokens : list, Index) -> tuple[bool, int]:
    assert(Index < len(Tokens)), "Invalid Conditional Expression Encountered"

    ExprRes = True
    while(Index < len(Tokens)):
        if Tokens[Index] == "defined":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'defined' statement"
            Index += 1
            assert(Tokens[Index] != ')' and Tokens[Index] != 'and' and Tokens[Index] != 'or'), "Keyword encounterd at wrong location"
            if Tokens[Index] == '(':
                (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
                ExprRes &= Res
            else:
                ExprRes &= TokenDictionary.get(Tokens[Index]) != None
                Index += 1

        elif Tokens[Index] == "and":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'and' statement"
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            ExprRes &= Res
        
        elif Tokens[Index] == "or":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'and' statement"
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            ExprRes |= Res
        
        elif Tokens[Index] == "not":
            assert(Index + 1 < len(Tokens)), "No Token Specified after 'and' statement"
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            Res = False
            ExprRes &= ~Res

        elif Tokens[Index] == '(':
            (Res, Index) = EvaluateBooleanExpression(Tokens, Index + 1)
            ExprRes &= Res

        elif Tokens[Index] == ')':
            return (ExprRes, Index)
        
        else: #Token is a variable to evaluate
            #TODO: Add Support for comparing two user defined tokens
            ExprRes &= TokenDictionary.get(Tokens[Index]) != None
            Index += 1



    return (ExprRes, Index)

def GetPreProcessorToken(Param, Index) -> tuple[str, int]:
    PreProcessorToken = ""
    while(Index < len(Param)) and (Param[Index] != ' ' and Param[Index] != '\n' and Param[Index] != '\r'):
        PreProcessorToken += Param[Index]
        Index += 1
    return (PreProcessorToken, Index)

def PrasePreprocessorBlock(Param, Index, PreProcessorToken) -> tuple[str, int]:
    #assert(Param[Index] == '#'), "Invalid PreProcessor Symbol Encountered"
    #Index = GetNextValidCharacterIndex(Param, Index)

    #assert len(Param) > Index and ord(Param[Index]) >= 97 and ord(Param[Index]) <= 122
    #(PreProcessorToken, Index) = GetPreProcessorToken(Param, Index)
    
    assert PreProcessorToken == "if" or \
            PreProcessorToken == "elif" or \
            PreProcessorToken == "else" or \
            PreProcessorToken == "endif", "Invalid PreProcessor Type encountered"
    
    #Accumulate Tokens until \n or \r is encountered
    if PreProcessorToken == "if" or PreProcessorToken == "elif":
        Index = GetNextValidCharacterIndex(Param, Index)
        BoolExpressionTokens = []
        while(Index < len(Param)) and Param[Index] != '\n' and Param[Index] != '\r':
            if Param[Index] == ' ' :
                Index = GetNextValidCharacterIndex(Param, Index)
            (Token, Index) = GetToken(Param, Index)
            if(Token != ""):
                BoolExpressionTokens.append(Token)


        (BoolRes, TupIndex) = EvaluateBooleanExpression(BoolExpressionTokens, 0)
        if BoolRes:
            return (Param, Index)        
        else:
            #If condition is false, jump the control to #else or #endif
            IfStack = []
            while Param[Index] != '#' or len(IfStack) != 0:
                Index = GetNextValidCharacterIndex(Param, Index)
                if(Param[Index] == '#'):
                    Index = GetNextValidCharacterIndex(Param, Index)
                    (PreProcessorToken, Index) = GetPreProcessorToken(Param, Index)
                    if(PreProcessorToken == "if"):
                        IfStack.append(PreProcessorToken)
                    elif (PreProcessorToken == "endif") and len(IfStack) != 0:
                        IfStack.pop()
                    elif len(IfStack) == 0:
                        break

            if PreProcessorToken == "elif" or PreProcessorToken == "else":
                return PrasePreprocessorBlock(Param, Index, "elif")
            else:
                return(Param, Index)
            
    #If control reaches the 'else' block, skip it entirely. else block will be handled within if, elif 
    elif PreProcessorToken == "else":
        IfStack = ["if"]
        while(PreProcessorToken != "endif") and len(IfStack) == 0:
            while(Token[Index] != '#'):
                Index += 1
                assert(Index != len(Param)), "#endif not encountered"
            (PreProcessorToken, Index) = GetPreProcessorToken(Param, Index)
            
            if(PreProcessorToken == "if"):
                IfStack.append("if")
            elif(PreProcessorToken == "endif"):
                IfStack.pop()    
        return(Param, Index)
    
    else:
        return(Param, Index)

    pass

def GetToken(Param, Index) -> tuple[str, int]:
    Token = ""
    assert Index < len(Param), "Invalid Index Passed"

    if Param[Index] == '(' or Param[Index] == ')':
        Token = Param[Index]
        Index += 1
        return Token

    while(Index < len(Param)) and Param[Index] != '\n' and Param[Index] != '\r' and Param[Index] != ' ':
        Token += Param[Index]
        Index += 1
    return (Token, Index)

def GetNextValidCharacterIndex(Param, Index) -> int:
    Index += 1
    while(Index < len(Param)) and (Param[Index] ==  ' ' or Param[Index] ==  '\n' or Param[Index] ==  '\r'):
        Index += 1
        continue
    return Index

def ParseNumber(Param, Index) -> tuple[int | float, int]:
    assert ord(Param[Index]) >= 48 and ord(Param[Index]) <= 57

    NumRes = Param[Index]
    while(ord(Param[Index + 1]) >= 48 and ord(Param[Index + 1]) <= 57) or (Param[Index + 1] == '.'):
        NumRes += Param[Index + 1]
        Index += 1

    if(NumRes.count('.') != 0):
        return (float(NumRes), Index)
    return (int(NumRes), Index)

def ParseString(Param, Index) -> tuple[str, int]:
    assert Param[Index] == '\"' or Param[Index] == '\''
    
    StringRes = ""
    Index += 1

    while(Param[Index] != '\"' and Param[Index] != '\''):
        StringRes += Param[Index]
        Index += 1
    
    return (StringRes, Index)

def ParseList(Param, Index) -> tuple[list, int]:
    assert Param[Index] == '['
    
    ListRes = []
    Index = GetNextValidCharacterIndex(Param, Index)
    while Param[Index] != ']':
        if Param[Index] == '\"' or Param[Index] == '\'':
            (Res, Index) = ParseString(Param, Index)
            ListRes.append(Res)
        
        elif Param[Index] == '[' or Param[Index] == '(':
            (Res, Index) = ParseList(Param, Index)
            ListRes.append(Res)
        
        elif Param[Index] == '{':
            (Res, Index) = ParseDictionary(Param, Index)
            ListRes.append(Res)
        
        elif ord(Param[Index]) >= 48 and ord(Param[Index]) <= 57:
            (Res, Index) = ParseNumber(Param, Index)
            ListRes.append(Res)
        
        elif Param[Index] == '#':
            Index = GetNextValidCharacterIndex(Param, Index)
            (Token, Index) = GetPreProcessorToken(Param, Index)
            (Param, Index) = PrasePreprocessorBlock(Param, Index, Token)
            (Param, Index) = GetNextValidCharacterIndex(Param, Index)
            continue
        else:
            assert False, "Invalid Symbol Encountered when Parsing List"

        Index = GetNextValidCharacterIndex(Param, Index)
        assert Param[Index] == ',' or Param[Index] == ']'
        if Param[Index] == ',':
            Index = GetNextValidCharacterIndex(Param, Index)
        pass

    return (ListRes, Index)

def ParseDictionary(Param, Index) -> tuple[dict, int]:
    assert Param[Index] == '{'
    
    DictRes = {}
    Index = GetNextValidCharacterIndex(Param, Index)
    while Param[Index] != '}':

        if Param[Index] == '#':
            Index = GetNextValidCharacterIndex(Param, Index)
            (Token, Index) = GetPreProcessorToken(Param, Index)
            (Param, Index) = PrasePreprocessorBlock(Param, Index, Token)
            Index = GetNextValidCharacterIndex(Param, Index)
            continue

        Key = ""
        if Param[Index] == '\"' or Param[Index] == '\'':
            (Key, Index) = ParseString(Param, Index)
            pass

        Index = GetNextValidCharacterIndex(Param, Index)
        assert Param[Index] == ':'
        Index = GetNextValidCharacterIndex(Param, Index)

        if Param[Index] == '\"' or Param[Index] == '\'':
            (Value, Index) = ParseString(Param, Index)
            DictRes[Key] = Value
            
        elif Param[Index] == '[' or Param[Index] == '(':
            (Value, Index) = ParseList(Param, Index)
            DictRes[Key] = Value
            
        elif Param[Index] == '{':
            (Value, Index) = ParseDictionary(Param, Index)
            DictRes[Key] = Value
            
        elif Param[Index] == '#':
            Index = GetNextValidCharacterIndex(Param, Index)
            (Token, Index) = GetPreProcessorToken(Param, Index)
            (Param, Index) = PrasePreprocessorBlock(Param, Index, Token)
            (Param, Index) = GetNextValidCharacterIndex(Param, Index)
            continue
        
        else:
            assert False, "Invalid Symbol Encountered when Parsing dictionary"

        Index = GetNextValidCharacterIndex(Param, Index)
        assert Param[Index] == ',' or Param[Index] == '}'
        if Param[Index] == ',':
            Index = GetNextValidCharacterIndex(Param, Index)

    return(DictRes, Index)


def ProcessParam(Param, Index) -> dict | list | str | None:
    (Type, Index, ResParam) = IdentifyParamTypeAndCleanup(Param, Index)
    Res = {}
    if Type == "Dictionary":
        (Res, Index) = ParseDictionary(ResParam, Index)
    elif Type == "List":
        (Res, Index) = ParseList(ResParam, Index)
    elif Type == "Number":
        (Res, Index) = ParseNumber(ResParam, Index)
    elif Type == "String":
        (Res, Index) = ParseString(ResParam, Index)
    else:
        assert False, "Trying to parse invalid type"

    return Res

def ReadTemplateFile(TemplateFilePath):
    if not os.path.isfile(TemplateFilePath):
        print("Encountered something that's not a file when trying to read template file")
        return None
    
    stream = open(TemplateFilePath, "r")
    contents = stream.read()
    stream.close()

    ModuleParamItr = re.finditer("(@:)(.*(\n|\r|\r\n))+?(.*:@)", contents)

    if(ModuleParamItr == None):
        print("No Module params found for this Module. This will skip CMakeLists generation")
    
    ParamDictionary = {}
    for Params in ModuleParamItr:
        CleanedStr = re.sub("(@:)|(:@)", "", Params.group())
        Index = GetNextValidCharacterIndex(CleanedStr, -1)
        while(Index < len(CleanedStr) and CleanedStr[Index] == '#'):
            Index = GetNextValidCharacterIndex(CleanedStr, Index)
            (Token, Index) = GetPreProcessorToken(CleanedStr, Index)
            (CleanedStr, Index) = PrasePreprocessorBlock(CleanedStr, Index, Token)
            Index = GetNextValidCharacterIndex(CleanedStr, Index)
        
        if(Index >= len(CleanedStr)):
            print(f"No settings retrieved from Build Template file at:{TemplateFilePath}. Check your defines")
            return ParamDictionary

        #CleanedStr = re.sub("(@:)|(:@)|[\n\r\s]", "", Params.group())
        #Remove New line and carriage return characters
        #CleanedStr = re.sub("(@:)|(:@)|[\n\r]", "", Params.group())
        #MyList = CleanedStr.split("=")
        (CleanedKey, Index) = ParseString(CleanedStr, Index)
        Index = GetNextValidCharacterIndex(CleanedStr, Index)
        assert CleanedStr[Index] == '=', "Not a valid entry"
        
        ParamDictionary[CleanedKey] = ProcessParam(CleanedStr, Index)
        #print(MyList)
        
    return ParamDictionary
