// +!periodic_update
//    <- .wait({+!send_update},10000,ETime);
//       if(ETime>9999){
//         !send_update;
//       }
//       !periodic_update.

//+!send_update(A)
//   <- getState(SchSt);
//      .send(A,update_sch,SchSt).

//+!send_update
//   <- getState(SchSt);
//      .broadcast(update_sch,SchSt).

//+commitment(A,_,_)
//  <-  .my_name(Me);
//      if(Me == A){
//        !send_update;
//      }.

// whenever some goal becomes satisfied, informs alice
//+goalState(S,_,_,_,satisfied)
//   <- .my_name(Me);
//      for (::commitment(A,_,S) & A \== Me) {
//         .print("send update to ",A)
//         !send_update(A);
//      }.

+!default::kqml_received(Sender, update_sch, SchSt, _)
   <- .print("Merging with state sent by ", Sender);
      this_ns::mergeState(SchSt);
      .print(Sender," state merged").

+!default::kqml_received(_, commit, Mission, _)
  <-  this_ns::commitMission(Mission).
