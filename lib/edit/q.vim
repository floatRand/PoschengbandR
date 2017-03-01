syn clear
syn case match
syn keyword qStatus Untaken Taken Completed Rewarded Finished Failed FailedDone contained
syn keyword qFeature FLOOR GRASS GRANITE PERMANENT TREE OPEN_DOOR CLOSED_DOOR UP_STAIR DOWN_STAIR 
syn keyword qFeature SHALLOW_WATER DEEP_WATER SHALLOW_LAVA DEEP_LAVA
syn keyword qFeature RUBBLE MOUNTAIN_WALL
syn keyword qFeature QUEST_ENTER ENTRANCE
syn keyword qExKeyword AND EQU LEQ GEQ NOT contained

syn match qBuilding /BUILDING_[0-9]*/
syn match qTrap /TRAP_[A-Z]*/
syn match qLockedDoor /LOCKED_DOOR_[0-9]*/
syn match qComment /^#.*$/
syn match qInclude /%:.*$/
syn match qVariable /$[A-Z0-9]*/ contained

syn region qExp matchgroup=qParen start=/\[/ end=/\]/ contains=qExp,qExKeyword,qVariable,qStatus contained
syn region qExpLine matchgroup=qExpPrefix start=/?:/ end=/$/ contains=qExp
syn region qObjExp matchgroup=qObj start=/OBJ(/ end=/)/ contained
syn region qEgoExp matchgroup=qEgo start=/EGO(/ end=/)/ contained
syn region qArtExp matchgroup=qArt start=/ART(/ end=/)/ contained
syn region qLetterLine matchgroup=qLetterPrefix start=/L:/ end=/$/ contains=qObjExp,qEgoExp,qArtExp


hi def link qStatus Special
hi def link qComment Comment
hi def link qFeature Constant
hi def link qTrap Constant
hi def link qLockedDoor Constant
hi def link qBuilding Constant
hi def link qInclude PreProc
hi def link qVariable Define
hi def link qKeyword Keyword
hi def link qExKeyword Keyword
hi def link qParen PreProc
hi def link qObj Constant
hi def link qEgo Type
hi def link qArt Identifier
hi def link qArtExp Special
hi def link qLetterPrefix Label
hi def link qExpPrefix Label

