+!periodic_update
   <- .random(X);
      .wait({+!send_update},14000+X*6000,ETime);
      if(ETime>13999+X*6000){
        !send_update;
      }
      !periodic_update.

+!send_update(A)
   <- getState(SchSt);
      .send(A,update_sch,SchSt).

+!send_update
   <- getState(SchSt);
      // for(connected(X)){
      //   .print("Sending to ", X);
      //   .wait(500);
      //   .send(X,update_sch,SchSt);
      // }
      .broadcast(update_sch,SchSt);
      .print("update Sent").

+commitment(A,_,_): .my_name(A)
   <- .print("Fiz commit!");
      !send_update;
      .

// whenever some goal becomes satisfied, informs alice
+goalState(S,_,_,_,satisfied)
   <- .my_name(Me);
      //for (::commitment(A,_,S) & A \== Me) {
      //   .print("********send update to ",A)
      //   !send_update(A);
      //}.
      !send_update;
      .



+!default::kqml_received(Sender, update_sch, SchSt, _)
   <- .print("Merging with state sent by ", Sender);
      this_ns::mergeState(SchSt);
      .print(Sender," state merged").

+!default::kqml_received(_, commit, Mission, _)
  <-  this_ns::commitMission(Mission).
