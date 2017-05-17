package de.fernuni_hagen.dna.jwh.secondopositiontransmitter.representation;

/**
 * Created by Jerome on 25.09.2015.
 */
public class ExtendedPosition  extends NLRepresentation {
        @Order(pos = 0)
        public String Id;
        @Order(pos = 1)
        public UPoint Position;
        @Order(pos = 2)
        public Double altitude;
        @Order(pos = 3)
        public Integer satellites;

        public ExtendedPosition() {
            Position = new UPoint();
            altitude = 0.0;
            satellites = 0;
        }

        @Override
        public String getSpecialType(String fieldName) {
            //No Switch-Statement, can only be Position
            return UPoint.getStaticType();
        }

        @Override
        public String getSpecialValue(String fieldName) {
            //No Switch-Statement, can only be Position
            return Position == null ? null : Position.getSecondoValue();
        }

}
