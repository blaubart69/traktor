/*
 * Copyright (c) 2014, Oracle America, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  * Neither the name of Oracle nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.sample;

import java.util.Optional;
import java.util.concurrent.TimeUnit;

import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.BenchmarkMode;
import org.openjdk.jmh.annotations.Measurement;
import org.openjdk.jmh.annotations.Mode;
import org.openjdk.jmh.annotations.Warmup;
import org.openjdk.jmh.infra.Blackhole;

import bumsti.BaselineSimple;
import bumsti.BaselineFloatMul;
import bumsti.CalcSettings;

public class Bm {

    @Benchmark
    @BenchmarkMode(Mode.SampleTime)
    @Warmup(iterations = 2)
    @Measurement(iterations = 1)
    public void runBaselineCalc(Blackhole blackhole) {

        final int screen_width = 960;
        final int screen_height = 720;

        CalcSettings calcSettings = new CalcSettings(
            screen_width / 2
            , screen_height
            , 1     // refSettings.rowPerspectivePx
            , 160   // refSettings.rowSpacingPx
            , 0     // refSettings.offset
            , 3     // row_count
        );

        int out_range = 0;
        int in_range = 0;
        long delta_pixels_sum = 0;
        long points = 0;

        long startTime = System.currentTimeMillis();

        for ( int f=0; f < 30; f++) 
        {
            for ( int x=0; x<screen_width;x++) 
            {
                for ( int y=0; y<screen_height;y++) 
                {
                    Integer delta_pixels = BaselineSimple.calc_baseline_delta_from_nearest_refline_simple(x,y, calcSettings);
                    if ( delta_pixels == null )
                    {
                        out_range += 1;
                    }
                    else
                    {
                        in_range += 1;
                        delta_pixels_sum += delta_pixels;
                    }
                    points += 1;
                }
            }
        }

        long elapsed = (System.currentTimeMillis() - startTime);

        //System.out.printf("ms: %d, delta_sum: %d, in: %d, out: %d, points: %d\n", elapsed, delta_pixels_sum, in_range, out_range, points);
        blackhole.consume(delta_pixels_sum);
        blackhole.consume(in_range);
        blackhole.consume(out_range);
        blackhole.consume(points);
    }
    @Benchmark
    @BenchmarkMode(Mode.SampleTime)
    @Warmup(iterations = 2)
    @Measurement(iterations = 1)
    public void runBaselineCalcFloatMul(Blackhole blackhole) {

        final int screen_width = 960;
        final int screen_height = 720;

        CalcSettings calcSettings = new CalcSettings(
            screen_width / 2
            , screen_height
            , 1     // refSettings.rowPerspectivePx
            , 160   // refSettings.rowSpacingPx
            , 0     // refSettings.offset
            , 3     // row_count
        );

        int out_range = 0;
        int in_range = 0;
        long delta_pixels_sum = 0;
        long points = 0;

        for ( int f=0; f < 30; f++) 
        {
            for ( int x=0; x<screen_width;x++) 
            {
                for ( int y=0; y<screen_height;y++) 
                {
                    Integer delta_pixels = BaselineFloatMul.calc_baseline_delta_from_nearest_refline_simple(x,y, calcSettings);
                    if ( delta_pixels == null )
                    {
                        out_range += 1;
                    }
                    else
                    {
                        in_range += 1;
                        delta_pixels_sum += delta_pixels;
                    }
                    points += 1;
                }
            }
        }

        //System.out.printf("ms: %d, delta_sum: %d, in: %d, out: %d, points: %d\n", elapsed, delta_pixels_sum, in_range, out_range, points);
        blackhole.consume(delta_pixels_sum);
        blackhole.consume(in_range);
        blackhole.consume(out_range);
        blackhole.consume(points);
    }

    @Benchmark
    @BenchmarkMode(Mode.SampleTime)
    @Warmup(iterations = 2)
    @Measurement(iterations = 1)
    public void runSimpleOptionalImpl(Blackhole blackhole) {

        final int screen_width = 960;
        final int screen_height = 720;

        CalcSettings calcSettings = new CalcSettings(
            screen_width / 2
            , screen_height
            , 1     // refSettings.rowPerspectivePx
            , 160   // refSettings.rowSpacingPx
            , 0     // refSettings.offset
            , 3     // row_count
        );

        int out_range = 0;
        int in_range = 0;
        long delta_pixels_sum = 0;
        long points = 0;

        var impl = new bumsti.BaselineSimpleOptionalImpl();

        for ( int f=0; f < 30; f++) 
        {
            for ( int x=0; x<screen_width;x++) 
            {
                for ( int y=0; y<screen_height;y++) 
                {
                    var delta_pixels = impl.DeltaToNearestRefLine(x,y, calcSettings);
                    if ( delta_pixels.isPresent() )
                    {
                        in_range += 1;
                        delta_pixels_sum += delta_pixels.get();
                    }
                    else                    {
                        out_range += 1;
                    }

                    points += 1;
                }
            }
        }

        //System.out.printf("ms: %d, delta_sum: %d, in: %d, out: %d, points: %d\n", elapsed, delta_pixels_sum, in_range, out_range, points);
        blackhole.consume(delta_pixels_sum);
        blackhole.consume(in_range);
        blackhole.consume(out_range);
        blackhole.consume(points);
    }
    @Benchmark
    @BenchmarkMode(Mode.SampleTime)
    @Warmup(iterations = 2)
    @Measurement(iterations = 1)
    public void runSimpleImpl(Blackhole blackhole) {

        final int screen_width = 960;
        final int screen_height = 720;

        CalcSettings calcSettings = new CalcSettings(
            screen_width / 2
            , screen_height
            , 1     // refSettings.rowPerspectivePx
            , 160   // refSettings.rowSpacingPx
            , 0     // refSettings.offset
            , 3     // row_count
        );

        int out_range = 0;
        int in_range = 0;
        long delta_pixels_sum = 0;
        long points = 0;

        var impl = new bumsti.BaselineSimpleImpl();

        for ( int f=0; f < 30; f++) 
        {
            for ( int x=0; x<screen_width;x++) 
            {
                for ( int y=0; y<screen_height;y++) 
                {
                    var delta_pixels = impl.DeltaToNearestRefLine(x,y, calcSettings);
                    if ( delta_pixels != null )
                    {
                        in_range += 1;
                        delta_pixels_sum += delta_pixels;
                    }
                    else                    {
                        out_range += 1;
                    }

                    points += 1;
                }
            }
        }

        //System.out.printf("ms: %d, delta_sum: %d, in: %d, out: %d, points: %d\n", elapsed, delta_pixels_sum, in_range, out_range, points);
        blackhole.consume(delta_pixels_sum);
        blackhole.consume(in_range);
        blackhole.consume(out_range);
        blackhole.consume(points);
    }
}
