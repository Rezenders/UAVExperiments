victim(-27.604593, -48.521134).
victim(-27.604531, -48.520998).
victim(-27.604522, -48.520952).

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

+!s1::registerPoints: s1::goalArgument(_,registerPoints,"N", N) & s1::goalArgument(_,registerPoints,"Lat1", Lat1) & s1::goalArgument(_,registerPoints,"Lon1", Lon1) & s1::goalArgument(_,registerPoints,"Lat2", Lat2) & s1::goalArgument(_,registerPoints,"Lon2", Lon2) & s1::goalArgument(_,registerPoints,"Lat3", Lat3) & s1::goalArgument(_,registerPoints,"Lon3", Lon3) & s1::goalArgument(_,registerPoints,"Lat4", Lat4) & s1::goalArgument(_,registerPoints,"Lon4", Lon4) & s1::goalArgument(_,registerPoints,"Lat5", Lat5) & s1::goalArgument(_,registerPoints,"Lon5", Lon5) & s1::goalArgument(_,registerPoints,"Lat6", Lat6) & s1::goalArgument(_,registerPoints,"Lon6", Lon6)
  <- +nPoints(N);
     +wayPoint(1,Lat1,Lon1);
     +wayPoint(2,Lat2,Lon2);
     +wayPoint(3,Lat3,Lon3);
     +wayPoint(4,Lat4,Lon4);
     +wayPoint(5,Lat5,Lon5);
     +wayPoint(6,Lat6,Lon6);
  .

+!s1::followRoute
  <- .wait(nPoints(N));
     ext::launch;
     .print("requested launch");
     .wait(status('flying'));
     .print("im flying");
     !searchVictims;
     for(.range(I,1,N)){
       !!goToP(I);
       .print("requested gotop");
       .wait(reached(I));
     }
  .

+!s1::returnHomeScout
  <- .print("Scout returning home");
     ext::returnHome;
     .wait(status(X) & X \== 'flying');
  .

+!s1::rescueDuty
  <- +buoys(1);
     +rescued(0);
  .

+!s1::returnHomeCourier
  <- .findall(Id,victimId(Id,_,_),L);
     .max(L,Nvictims);
     .wait(rescued(Nrescued) & Nrescued>Nvictims);
     if(status(X) & X=='flying'){
      .print("Courier returning home");
      ext::returnHome;
     }
     .wait(status(S) & S \== 'flying');
     .print("Courier home");
  .

+!goToP(N): wayPoint(N,Lat,Lon)
  <- .print("Going to P at ",Lat," ",Lon," ",10.0);
     ext::setWaypoint(Lat,Lon,10.0);
     !!checkPos(N,Lat,Lon,10.0);
  .

+!checkPos(N,WPLat,WPLon,WPAlt): pos(Lat,Lon,Alt) & jia.gps_dist(WPLat,WPLon,WPAlt,Lat,Lon,Alt,D) & D<3
  <- -+reached(N).

+!checkPos(N,WPLat,WPLon,WPAlt)
  <- .wait(2000);
     !checkPos(N,WPLat,WPLon,WPAlt).

+!searchVictims
  <- +nVictims(0);
     for(victim(Lat,Lon)){
        !!checkVictim(Lat,Lon);
     }
  .

+!checkVictim(VLat,VLon): pos(Lat,Lon,Alt) & jia.gps_dist(VLat,VLon,10.0,Lat,Lon,Alt,D) & D<3 & nVictims(N)
  <- .send(courier,achieve,registerVictim(N,VLat,VLon));
     -+nVictims(N+1);
  .

+!checkVictim(VLat,VLon)
  <- .wait(2000);
     !checkVictim(VLat,VLon);
  .

+!registerVictim(Id,VLat, VLon)
  <- +victimId(Id,VLat,VLon);
     !!rescueVictim(Id);
  .

+!rescueVictim(Id): victimId(Id,Lat,Lon)
  <- .wait(rescued(Id));
     !getBuoy;
     if(status(X) & X \== 'flying'){
       ext::launch;
       .wait(status('flying'));
     }
     ext::setWaypoint(Lat,Lon,20.0);
     !!checkPos(Id,Lat,Lon,20.0);
     .wait(reached(Id));
     .print("Delivering Buoy!");
     -+buoys(0);
     -+rescued(Id+1);
  .

+!getBuoy: buoys(1).

+!getBuoy
  <- if(status(X) & X \== 'notReady'){
      ext::returnHome;
     }
     .wait(status('notReady'));
     -+buoys(1);
     !getBuoy;
  .

{ include("sync-sch.asl", s1) }

{ include("$jacamoJar/templates/common-cartago.asl") }
{ include("$jacamoJar/templates/common-moise.asl") }
{ include("$jacamoJar/templates/org-obedient.asl", s1) }
