!start.

+!start: connected(A) & connected(B) & A \== B
   <- createWorkspace(wa);
      joinWorkspace(wa,O4MWsp);

      makeArtifact(myorg, "ora4mas.nopl.OrgBoard", ["src/org/org.xml"], OrgArtId)[wid(O4MWsp)];
      tmporg::focus(OrgArtId);
      tmporg::createScheme(s2, scheme1, SchArtId)[wid(O4MWsp)];
      s2ns::focus(SchArtId)[wid(O4MWsp)];
      //tmporg::debug(inspector_gui(on));

      .send(scout,achieve,setup);
      .wait(1000);
      .send(courier,achieve,setup);
      !commitAll
    .

+!start
  <-  .wait(1000);
      !start;
  .

+!commitAll: s2ns::commitment(X,_,_) & s2ns::commitment(Y,_,_).

+!commitAll
  <- .print("Still not commited");
     .wait(10000);
     .send(scout,commit,missionScout);
     .wait(1000);
     .send(courier,commit,missionCourier);
     !commitAll;
  .

+s2ns::goalState(_,goal1,_,_,satisfied)
   <- .print("*** all done! ***").

{ include("sync-sch.asl", s2ns) }

{ include("$jacamoJar/templates/common-cartago.asl") }
{ include("$jacamoJar/templates/common-moise.asl") }
{ include("$jacamoJar/templates/org-obedient.asl", s2ns) }
