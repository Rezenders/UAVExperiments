
+!setup
   <- createWorkspace(wb);
      joinWorkspace(wb,O4MWsp);

      makeArtifact(myorg, "ora4mas.nopl.OrgBoard", ["src/org/org.xml"], OrgArtId)[wid(O4MWsp)];
      tmporg::focus(OrgArtId);
      tmporg::createScheme(s1, scheme1, SchArtId)[wid(O4MWsp)];
      s1::focus(SchArtId)[wid(O4MWsp)];
      //tmporg::debug(inspector_gui(on));
      .wait(500);
      !s1::periodic_update;
   .

+!s1::goToP1: s1::goalArgument(_,goToP1,"Lat",Lat) & s1::goalArgument(_,goToP1,"Lon",Lon) & s1::goalArgument(_,goToP1,"Alt",Alt)
 <- .print("1-Going to P at ",Lat," ",Lon," ",Alt);
    -reached(Lat,Lon,Alt);
    ext::launch;
    .wait(5000);
    ext::setWaypoint(Lat,Lon,Alt);
    !checkPos(Lat,Lon,Alt);
    .wait(reached(Lat,Lon,Alt));
 .

+!s1::goToP2: s1::goalArgument(_,goToP2,"Lat",Lat) & s1::goalArgument(_,goToP2,"Lon",Lon) & s1::goalArgument(_,goToP2,"Alt",Alt)
 <- .print("2-Going to P at ",Lat," ",Lon," ",Alt);
    -reached(Lat,Lon,Alt);
    ext::launch;
    .wait(5000);
    ext::setWaypoint(Lat,Lon,Alt);
    !checkPos(Lat,Lon,Alt);
    .wait(reached(Lat,Lon,Alt));
 .

+!s1::returnHome1
 <- .print("1-Returning Home");
    ext::returnHome;
    .wait(status('ready'));
 .

+!s1::returnHome2
 <- .print("2-Returning Home");
    ext::returnHome;
    .wait(status('ready'));
 .

+!checkPos(WPLat,WPLon,WPAlt): pos(Lat,Lon,Alt) & jia.gps_dist(WPLat,WPLon,WPAlt,Lat,Lon,Alt,D) & D<5
 <- +reached(WPLat,WPLon,WPAlt).

+!checkPos(WPLat,WPLon,WPAlt)
 <- .wait(1000);
    !checkPos(WPLat,WPLon,WPAlt).

{ include("sync-sch.asl", s1) }

{ include("$jacamoJar/templates/common-cartago.asl") }
{ include("$jacamoJar/templates/common-moise.asl") }
{ include("$jacamoJar/templates/org-obedient.asl", s1) }
