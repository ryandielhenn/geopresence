package geogrid;

import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

public class QueryBenchmark {

    public QueryBenchmark() {}

    public void queryBenchmark(int iterations) throws Exception {

        GeoavailabilityGrid gg = new GeoavailabilityGrid("9x", 30);

        float minLat = gg.getBaseRange().getLowerBoundForLatitude();
        float maxLat = gg.getBaseRange().getUpperBoundForLatitude();
        float minLon = gg.getBaseRange().getLowerBoundForLongitude();
        float maxLon = gg.getBaseRange().getUpperBoundForLongitude();

        for (int i = 0; i < iterations; ++i) {
            float randomLat
                = minLat + (float) Math.random() * (maxLat - minLat);

            float randomLon
                = minLon + (float) Math.random() * (maxLon - minLon);

            gg.addPoint(new Coordinates(randomLat, randomLon));
        }


        List<Coordinates> p1 = new ArrayList<>();
        p1.add(new Coordinates(44.919f, -112.242f));
        p1.add(new Coordinates(43.111f, -105.414f));
        p1.add(new Coordinates(41.271f, -111.421f));
        GeoavailabilityQuery q1 = new GeoavailabilityQuery(p1);

        long startTime = System.nanoTime();
        gg.intersects(q1);
        System.out.println((System.nanoTime() - startTime) / 1000000000.0);
    }

    public static void main(String[] args) throws Exception {
        Logger rootLogger = Logger.getLogger("");
        rootLogger.setLevel(Level.OFF);
        QueryBenchmark qb = new QueryBenchmark();
        qb.queryBenchmark(Integer.parseInt(args[0]));
    }
}
