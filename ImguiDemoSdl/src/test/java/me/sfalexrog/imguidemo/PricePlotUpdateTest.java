package me.sfalexrog.imguidemo;

import org.json.JSONObject;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;

public class PricePlotUpdateTest {
    private void updatePlot(List<Double> x, List<Double> prices, String msg, int maxPoints) throws Exception {
        JSONObject j = new JSONObject(msg);
        double price = j.getDouble("p");
        prices.add(price);
        x.add((double) prices.size());
        if (prices.size() > maxPoints) {
            prices.remove(0);
            x.remove(0);
        }
    }

    @Test
    public void addsPriceToPlot() throws Exception {
        List<Double> prices = new ArrayList<>();
        List<Double> x = new ArrayList<>();
        updatePlot(x, prices, "{\"p\":\"100.5\"}", 5);
        assertEquals(1, prices.size());
        assertEquals(100.5, prices.get(0), 0.001);
        assertEquals(1.0, x.get(0), 0.001);
    }

    @Test
    public void respectsMaxPoints() throws Exception {
        List<Double> prices = new ArrayList<>();
        List<Double> x = new ArrayList<>();
        int max = 2;
        updatePlot(x, prices, "{\"p\":\"1\"}", max);
        updatePlot(x, prices, "{\"p\":\"2\"}", max);
        updatePlot(x, prices, "{\"p\":\"3\"}", max);
        assertEquals(max, prices.size());
        assertEquals(2.0, x.get(0), 0.001);
        assertEquals(3.0, x.get(1), 0.001);
    }
}
