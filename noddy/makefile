
all:      noddy.exe 


CCOPTS      = -O0 -w -g
CC        = gcc
LD      = gcc

#
#        Utility Definitions
#
COPY_CMD  = cp
DEL_CMD   = rm
MOVE_CMD  = mv

%.o: %.c
	$(CC) -c -o $@ $< $(CCOPTS)


COPY_CMD  = cp
DEL_CMD   = rm
MOVE_CMD  = rmven


clean:
	- $(DEL_CMD) *.o
	- $(DEL_CMD) noddy.exe
	
#
#        List the project object modules
#
OBJECTFILES1 = testNod.o noddy.o color.o profile.o geolOp.o threedOp.o
OBJECTFILES2 = lineMap.o projOp.o geophOp.o jobStat.o topoOp.o
OBJECTFILES3 = aboutBox.o parOp.o movieOp.o convert.o xyzgen.o
OBJECTFILES4 = xyzConvt.o rockprop.o profOpt.o eventwin.o prevwin.o
OBJECTFILES5 = xyzwin.o ddpwin.o rotwin.o ppwin.o gmawin.o
OBJECTFILES6 = altwin.o dddwin.o matwin.o rockwin.o fformwin.o
OBJECTFILES7 = fdsclwin.o tgmawin.o fsclwin.o ssclwin.o dformwin.o
OBJECTFILES8 = swwin.o pformwin.o psclwin.o axiswin.o cornwin.o
OBJECTFILES9 = ipropwin.o isclwin.o surfwin.o foldtwin.o falttwin.o
OBJECTFILES10 = tracewin.o stratwin.o generwin.o timewin.o prevmenu.o
OBJECTFILES11 = licwin.o imopwin.o infowin.o viewop.o blklayop.o
OBJECTFILES12 = poswin.o refwin.o dxfinwin.o apropwin.o filefwin.o
OBJECTFILES13 = optwin.o chairwin.o thredwin.o newwin.o genwin.o
OBJECTFILES14 = fieldwin.o altitwin.o drapewin.o rangewin.o ctypewin.o
OBJECTFILES15 = defmwin.o padwin.o vcubewin.o sunitwin.o imsclwin.o
OBJECTFILES16 = imdiswin.o lrwin.o unitwin.o ptsclwin.o wposwin.o
OBJECTFILES17 = vnamewin.o csizewin.o orignwin.o dimwin.o utopowin.o
OBJECTFILES18 = fognwin.o moviewin.o borewin.o sectwin.o bgcolwin.o
OBJECTFILES19 = surfawin.o volwin.o helpwin.o mainMenu.o lineMenu.o
OBJECTFILES20 = batchNod.o builder.o rmbuild.o nodLib1.o nodLib2.o
OBJECTFILES21 = nodLib3.o nodGraph.o nodwork1.o nodwork2.o lineEvnt.o
OBJECTFILES22 = DoBlock.o DoSect.o DoSectLn.o DoLMap.o DoLMapLn.o
OBJECTFILES23 = SetUpMem.o dotmap3.o smothmap.o ldotmap.o ldotmap2.o
OBJECTFILES24 = mcontr.o plot.o DrawRut.o calc3d.o utils.o
OBJECTFILES25 = unEvents.o events.o taste.o which.o ellFault.o
OBJECTFILES26 = block3.o lblock3.o extras.o ludcmp.o lubksb.o
OBJECTFILES27 = dipcal.o getStat.o getHist.o getEvent.o getput.o
OBJECTFILES28 = readLine.o reporter.o putEvent.o block.o geophy.o
OBJECTFILES29 = Macmag3.o calcanom.o ERMapper.o DrawCurv.o remProc.o
OBJECTFILES30 = 3d.o 3dBitmap.o 3dCamera.o 3dClip.o 3dColor.o
OBJECTFILES31 = 3dCube.o 3dFMath.o 3dGrid.o 3dHermit.o 3dHier.o
OBJECTFILES32 = 3dLight.o 3dLinkL.o 3dMatPat.o 3dNTDraw.o 3dQuad.o
OBJECTFILES33 = 3dRender.o 3dRes.o 3dSolPgn.o 3dSolids.o 3dSpline.o
OBJECTFILES34 = 3dSurf.o 3dText.o 3dTextur.o 3dXFDraw.o AlphaTet.o
OBJECTFILES35 = BetaTet.o GammaTet.o DeltaTet.o EpsilTet.o allSurf.o
OBJECTFILES36 = dxfLib.o magCalc.o fft.o vulcan.o graph.o
OBJECTFILES37 = proflib.o eventlib.o dxfline.o dxfmisc.o dxfread.o
OBJECTFILES38 = dxfsconv.o dxfsout.o dxfvconv.o dxfvout.o dxfvox.o
OBJECTFILES39 = matrix.o geosoft.o vderiv.o import.o optnlib.o
OBJECTFILES40 = export.o legend.o
OBJECTFILES = $(OBJECTFILES1) $(OBJECTFILES2) $(OBJECTFILES3) $(OBJECTFILES4) $(OBJECTFILES5) $(OBJECTFILES6) $(OBJECTFILES7) $(OBJECTFILES8) $(OBJECTFILES9) $(OBJECTFILES10) $(OBJECTFILES11) $(OBJECTFILES12) $(OBJECTFILES13) $(OBJECTFILES14) $(OBJECTFILES15) $(OBJECTFILES16) $(OBJECTFILES17) $(OBJECTFILES18) $(OBJECTFILES19) $(OBJECTFILES20) $(OBJECTFILES21) $(OBJECTFILES22) $(OBJECTFILES23) $(OBJECTFILES24) $(OBJECTFILES25) $(OBJECTFILES26) $(OBJECTFILES27) $(OBJECTFILES28) $(OBJECTFILES29) $(OBJECTFILES30) $(OBJECTFILES31) $(OBJECTFILES32) $(OBJECTFILES33) $(OBJECTFILES34) $(OBJECTFILES35) $(OBJECTFILES36) $(OBJECTFILES37) $(OBJECTFILES38) $(OBJECTFILES39) $(OBJECTFILES40)

#
#        List the project dependencies
#
noddy.o: nodInc.h

color.o: nodInc.h

profile.o: nodInc.h

geolOp.o: nodInc.h

threedOp.o: nodInc.h

lineMap.o: nodInc.h

projOp.o: nodInc.h

geophOp.o: nodInc.h

jobStat.o: nodInc.h

topoOp.o: nodInc.h

aboutBox.o: nodInc.h

parOp.o: nodInc.h

movieOp.o: nodInc.h

convert.o: nodInc.h

xyzgen.o: nodInc.h

xyzConvt.o: nodInc.h

rockprop.o: nodInc.h

profOpt.o: nodInc.h

eventwin.o: nodInc.h

prevwin.o: nodInc.h

xyzwin.o: nodInc.h

ddpwin.o: nodInc.h

rotwin.o: nodInc.h

ppwin.o: nodInc.h

gmawin.o: nodInc.h

altwin.o: nodInc.h

dddwin.o: nodInc.h

matwin.o: nodInc.h

rockwin.o: nodInc.h

fformwin.o: nodInc.h

fdsclwin.o: nodInc.h

tgmawin.o: nodInc.h

fsclwin.o: nodInc.h

ssclwin.o: nodInc.h

dformwin.o: nodInc.h

swwin.o: nodInc.h

pformwin.o: nodInc.h

psclwin.o: nodInc.h

axiswin.o: nodInc.h

cornwin.o: nodInc.h

ipropwin.o: nodInc.h

isclwin.o: nodInc.h

surfwin.o: nodInc.h

foldtwin.o: nodInc.h

falttwin.o: nodInc.h

tracewin.o: nodInc.h

stratwin.o: nodInc.h

generwin.o: nodInc.h

timewin.o: nodInc.h

prevmenu.o: nodInc.h

licwin.o: nodInc.h

imopwin.o: nodInc.h

infowin.o: nodInc.h

viewop.o: nodInc.h

blklayop.o: nodInc.h

poswin.o: nodInc.h

refwin.o: nodInc.h

dxfinwin.o: nodInc.h

apropwin.o: nodInc.h

filefwin.o: nodInc.h

optwin.o: nodInc.h

chairwin.o: nodInc.h

thredwin.o: nodInc.h

newwin.o: nodInc.h

genwin.o: nodInc.h

fieldwin.o: nodInc.h

altitwin.o: nodInc.h

drapewin.o: nodInc.h

rangewin.o: nodInc.h

ctypewin.o: nodInc.h

defmwin.o: nodInc.h

padwin.o: nodInc.h

vcubewin.o: nodInc.h

sunitwin.o: nodInc.h

imsclwin.o: nodInc.h

imdiswin.o: nodInc.h

lrwin.o: nodInc.h

unitwin.o: nodInc.h

ptsclwin.o: nodInc.h

wposwin.o: nodInc.h

vnamewin.o: nodInc.h

csizewin.o: nodInc.h

orignwin.o: nodInc.h

dimwin.o: nodInc.h

utopowin.o: nodInc.h

fognwin.o: nodInc.h

moviewin.o: nodInc.h

borewin.o: nodInc.h

sectwin.o: nodInc.h

bgcolwin.o: nodInc.h

surfawin.o: nodInc.h

volwin.o: nodInc.h

helpwin.o: nodInc.h

mainMenu.o: nodInc.h

lineMenu.o: nodInc.h

batchNod.o: noddy.h

builder.o: rmbuild.c

rmbuild.o: builder.h

nodLib1.o: noddy.h

nodLib2.o: noddy.h

nodLib3.o: noddy.h

nodGraph.o: noddy.h color.h builder.h nodStruc.h

nodwork1.o: noddy.h

nodwork2.o: noddy.h

lineEvnt.o: noddy.c

DoBlock.o: nodStruc.h

DoSect.o: nodStruc.h

DoSectLn.o: noddy.h

DoLMap.o: noddy.h

DoLMapLn.o: noddy.h

SetUpMem.o: noddy.h

dotmap3.o: noddy.h

smothmap.o: noddy.h

ldotmap.o: noddy.h

ldotmap2.o: noddy.h

mcontr.o: noddy.h

plot.o: noddy.h scales.h

DrawRut.o: noddy.h

calc3d.o: noddy.h 3d.h

utils.o: nodStruc.h

unEvents.o: nodStruc.h

events.o: noddy.h

taste.o: noddy.h

which.o: noddy.h

ellFault.o: noddy.h nodStruc.h

block3.o: noddy.h nodStruc.h

lblock3.o: noddy.h

extras.o: noddy.h nodStruc.h

ludcmp.o: noddy.h

lubksb.o: noddy.h

dipcal.o: noddy.h

getStat.o: noddy.h

getHist.o: noddy.h

getEvent.o: noddy.h

getput.o: noddy.h

readLine.o: noddy.h

reporter.o: noddy.h

putEvent.o: noddy.h

block.o: noddy.h

geophy.o: noddy.h

Macmag3.o: noddy.h

calcanom.o: noddy.h

ERMapper.o: noddy.h

DrawCurv.o: noddy.h

remProc.o: noddy.h

3d.o: 3d.h

3dBitmap.o: 3dBitmap.h

3dCamera.o: 3dCamera.h

3dClip.o: 3dClip.h

3dColor.o: 3dColor.h

3dCube.o: 3dCube.h

3dFMath.o: 3dFMath.h

3dGrid.o: 3dGrid.h

3dHermit.o: 3dHermit.h

3dHier.o: 3dHier.h

3dLight.o: 3dLight.h

3dLinkL.o: 3dLinkL.h

3dMatPat.o: 3dMatPat.h

3dNTDraw.o: 3dNTDraw.h

3dQuad.o: 3dQuad.h

3dRender.o: 3dRender.h

3dRes.o: 3dRes.h

3dSolPgn.o: 3dSolPgn.h

3dSolids.o: 3dSolids.h

3dSpline.o: 3dSpline.h

3dSurf.o: 3dSurf.h

3dText.o: 3dText.h

3dTextur.o: 3dTextur.h

3dXFDraw.o: 3dXFDraw.h

AlphaTet.o: allSurf.h

BetaTet.o: allSurf.h

GammaTet.o: allSurf.h

DeltaTet.o: allSurf.h

EpsilTet.o: allSurf.h

allSurf.o: allSurf.h

dxfLib.o: allSurf.h

magCalc.o: noddy.h

fft.o: noddy.h

vulcan.o: noddy.h

graph.o: noddy.h

proflib.o: noddy.h

eventlib.o: noddy.h

dxfline.o: dxf.h

dxfmisc.o: dxf.h

dxfread.o: dxf.h

dxfsconv.o: dxf.h

dxfsout.o: dxf.h

dxfvconv.o: dxf.h

dxfvout.o: dxf.h

dxfvox.o: dxf.h

matrix.o: dxf.h

geosoft.o: noddy.h

vderiv.o: noddy.h

import.o: noddy.h

optnlib.o: noddy.h

export.o: noddy.h

legend.o: noddy.h

#
#        Application link line
#
noddy.exe: $(OBJECTFILES) 
	$(CC) $(OBJECTFILES)  -o $@
