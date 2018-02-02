// Internal action code for project gps_test

package jia;

import jason.*;
import jason.asSemantics.*;
import jason.asSyntax.*;

public class gps_dist extends DefaultInternalAction {

    @Override
    public Object execute(TransitionSystem ts, Unifier un, Term[] args) throws Exception {
        // execute the internal action

    	  double lat1 = Double.parseDouble(args[0].toString());
        double lon1 = Double.parseDouble(args[1].toString());
        double el1 = Double.parseDouble(args[2].toString());
    	  double lat2 = Double.parseDouble(args[3].toString());
        double lon2 = Double.parseDouble(args[4].toString());
        double el2 = Double.parseDouble(args[5].toString());


    	  final int R = 6371; // Radius of the earth

        double latDistance = Math.toRadians(lat2 - lat1);
        double lonDistance = Math.toRadians(lon2 - lon1);
        double a = Math.sin(latDistance / 2) * Math.sin(latDistance / 2)
                + Math.cos(Math.toRadians(lat1)) * Math.cos(Math.toRadians(lat2))
                * Math.sin(lonDistance / 2) * Math.sin(lonDistance / 2);
        double c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
        double distance = R * c * 1000; // convert to meters

        double height = el2 - el1;

        distance = Math.sqrt(Math.pow(distance, 2) + Math.pow(height, 2));
        //ts.getAg().getLogger().info("distance: " + distance);
        return un.unifies(args[6], new NumberTermImpl(distance));
    }
}
