+!cnp(Item): !cfp(Item,_) | getBid()

+!cnp(Item)
  <-  .my_name(Me);
      .broadcast(achieve, cfp(Item,Me));
      !cfp(Item,Me);
      .wait(5000); //wait 5 seconds to close auction
      !selectWinner(Item).

+!cfp(Item,_): cfp(Item).

+!cfp(Item,Agent)
  <-  +cfp(Item);
      !getBid(Item,Agent).

-getBid(Item,Agent)
  <-  .my_name(Me);
      if(Me == Agent){
        +proposal(0,Item,Me);
      } else {
        .send(Agent,tell,proposal(0,Item,Me));
      }
      .

+!selectWinner(Item)
  <-  .findall(proposal(Value,Item,Agent),proposal(Value,Item,Agent),L);
      .max(L,proposal(V,I,A));
      .my_name(Me);
      if(Me == A){
        +contract(Item);
      } else {
        .send(A,tell,contract(Item));
      }
      .abolish(proposal(_,I,_)).

-!selectWinnet(Item).
