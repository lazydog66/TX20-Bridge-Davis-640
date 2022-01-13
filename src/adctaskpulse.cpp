    class Pulsar {

        // Construct the pulse filter.
        //
        // leadWidth - minimum lead time before a pulse, measure in samples count
        // pulseWidth - minimimum width of the pulse, measured in samples count
        // lowLevel - threshold for logic low, range [0, 255]
        //
        // debounce ->|<- min pulse width ->|
        // ************                     **********
        //            *                     *
        //            *                     *
        //            ***********************
        //
        constructor(pulseWidth, debounceWidth, lowLevel, movingAverageMax, pulseFn) {
            this.pulseWidth = pulseWidth;
            this.debounceWidth = debounceWidth;
            this.lowLevel = lowLevel;
            this.movingAverageMax = movingAverageMax;
            this.pulseFn = pulseFn;

            // Keep track of the sample indices.
            this.index = 0;

            // Keep track of the index of the last pulse, used for debounce.
            this.pulseIndex = 0;

            // The moving averages for the pulse lead in, ie the 1st run of * in the pulse below.
            this.movingAverage = new MovingAverage(this.pulseWidth);
        }

        //
        // Clear the filter ready for a new session.
        //
        clear() {
            this.index = 0;
            this.pulseIndex = 0;
            this.movingAverage.clear();
        }

        //
        // Accept a new sample.
        //
        processSample(value) {

            // Clean up the sample value.
            // Note the moving average uses inverted logic.
            value = value <= this.lowLevel ? 1 : 0;

            // Add this sample into the moving average buffer.
            this.movingAverage.push(value);

            // Calculate the moving average, if it's equal to one then we've detected a pulse.
            const average = this.movingAverage.average();
            const havePulse = average >= this.movingAverageMax;

            if (havePulse) {
                if (this.index - this.pulseIndex >= this.debounceWidth)
                    if (this.pulseFn) this.pulseFn(this.index);

                this.pulseIndex = this.index;
            }

            ++this.index;
        }
    }