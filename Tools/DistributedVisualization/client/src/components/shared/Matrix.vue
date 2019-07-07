<template>
  <v-container grid-list-xl>
    <v-layout align-center>
      <div
        v-if="viewOrientation === 'vertical'"
        id="svg_container_matrix"
        class="svg_matrix_container"
        :style="paddingBottomStyle"
      >
        <svg
          v-if="isCollect2 === true"
          class="svg_matrix"
          preserveAspectRatio="xMaxYMax meet"
          :viewBox="viewBox"
          xmlns="http://www.w3.org/2000/svg"
          xmlns:xlink="http://www.w3.org/1999/xlink"
        >
          <g v-for="(worker, index_w) in matrixData" :key="index_w">
            <g>
              <rect
                :x="0 + index_w*(rectDimension+rectOffsetX)"
                :y="0"
                :width="rectDimension"
                :height="rectDimension*collect2FirstStepSizeFactor"
                :fill="rectFillFinished"
              />
              <rect
                v-if="worker.slots[0] !== 'finished'"
                :x="0 + index_w*(rectDimension+rectOffsetX) + strokeWidth/2"
                :y="strokeWidth/2"
                :width="rectDimension - strokeWidth"
                :height="rectDimensionForRunning(worker.runningProgress, worker.slots, 0, false)*collect2FirstStepSizeFactor - strokeWidth"
                :fill="rectFillNotFinished"
              />
            </g>
            <g v-for="(index) of numOfSlotsPerWorker" :key="index">
              <rect
                :x="0 + index_w*(rectDimension+rectOffsetX)"
                :y="rectDimension*5 + rectDimension*2 + index*(rectDimension+rectOffsetY)"
                :width="rectDimension"
                :height="rectDimension"
                :fill="rectFillFinished"
              />
              <rect
                v-if="worker.slots[1] !== 'finished'"
                :x="0 + index_w*(rectDimension+rectOffsetX) + strokeWidth/2"
                :y="rectDimension*5 + rectDimension*2 + index*(rectDimension+rectOffsetY) + strokeWidth/2"
                :width="rectDimension - strokeWidth"
                :height="rectDimensionForRunning(worker.runningProgress, worker.slots, index, true) - strokeWidth"
                :fill="rectFillNotFinished"
              />
            </g>
          </g>
        </svg>
        <svg
          v-else
          class="svg_matrix"
          preserveAspectRatio="xMaxYMax meet"
          :viewBox="viewBox"
          xmlns="http://www.w3.org/2000/svg"
          xmlns:xlink="http://www.w3.org/1999/xlink"
        >
          <g v-for="(worker, index_w) in matrixData" :key="index_w">
            <g v-for="(slot, index) in worker.slots" :key="index">
              <rect
                :x="0 + index_w*(rectDimension+rectOffsetX)"
                :y="0 + index*(rectDimension+rectOffsetY)"
                :width="rectDimension"
                :height="rectDimension"
                :fill="rectFillFinished"
              />
              <rect
                v-if="slot !== 'finished'"
                :x="0 + index_w*(rectDimension+rectOffsetX) + strokeWidth/2"
                :y="0 + index*(rectDimension+rectOffsetY) + strokeWidth/2"
                :width="rectDimension - strokeWidth"
                :height="rectDimensionForRunning(worker.runningProgress, worker.slots, index, false) - strokeWidth"
                :fill="rectFillNotFinished"
              />
            </g>
          </g>
        </svg>
      </div>
      <div
        v-if="viewOrientation === 'horizontal'"
        id="svg_container_matrix"
        class="svg_matrix_container"
        :style="paddingBottomStyle"
      >
        <svg
          v-if="isCollect2 === true"
          class="svg_matrix"
          preserveAspectRatio="xMaxYMax meet"
          :viewBox="viewBox"
          xmlns="http://www.w3.org/2000/svg"
          xmlns:xlink="http://www.w3.org/1999/xlink"
        >
          <g v-for="(worker, index_w) in matrixData" :key="index_w">
            <g>
              <rect
                :y="0 + index_w*(rectDimension+rectOffsetX)"
                :x="0"
                :width="rectDimension*collect2FirstStepSizeFactor"
                :height="rectDimension"
                :fill="rectFillFinished"
              />
              <rect
                v-if="worker.slots[0] !== 'finished'"
                :y="0 + index_w*(rectDimension+rectOffsetX) + strokeWidth/2"
                :x="0 + 0*(rectDimension+rectOffsetY) + strokeWidth/2"
                :width="rectDimension*5 - strokeWidth"
                :height="rectDimension - strokeWidth"
                :fill="rectFillNotFinished"
              />
              <rect
                v-if="worker.slots[0] !== 'finished'"
                :y="0 + index_w*(rectDimension+rectOffsetX)"
                :x="0"
                :width="rectDimensionForRunning(worker.runningProgress, worker.slots, 0, false)*collect2FirstStepSizeFactor"
                :height="rectDimension"
                :fill="rectFillFinished"
              />
            </g>
            <g v-for="(index) of numOfSlotsPerWorker" :key="index">
              <rect
                :y="0 + index_w*(rectDimension+rectOffsetX)"
                :x="rectDimension*5 + rectDimension*2 + index*(rectDimension+rectOffsetY)"
                :width="rectDimension"
                :height="rectDimension"
                :fill="rectFillFinished"
              />
              <rect
                v-if="worker.slots[1] !== 'finished'"
                :y="0 + index_w*(rectDimension+rectOffsetX) + strokeWidth/2"
                :x="rectDimension*5 + rectDimension*2 + index*(rectDimension+rectOffsetY) + strokeWidth/2"
                :width="rectDimension - strokeWidth"
                :height="rectDimensionForRunning(worker.runningProgress, worker.slots, index, true) - strokeWidth"
                :fill="rectFillNotFinished"
              />
            </g>
          </g>
        </svg>
        <svg
          v-else
          class="svg_matrix"
          preserveAspectRatio="xMaxYMax meet"
          :viewBox="viewBox"
          xmlns="http://www.w3.org/2000/svg"
          xmlns:xlink="http://www.w3.org/1999/xlink"
        >
          <g v-for="(worker, index_w) in matrixData" :key="index_w">
            <g v-for="(slot, index) in worker.slots" :key="index">
              <rect
                :y="0 + index_w*(rectDimension+rectOffsetX)"
                :x="0 + index*(rectDimension+rectOffsetY)"
                :width="rectDimension"
                :height="rectDimension"
                :fill="rectFillFinished"
              />
              <rect
                v-if="slot !== 'finished'"
                :y="0 + index_w*(rectDimension+rectOffsetX) + strokeWidth/2"
                :x="0 + index*(rectDimension+rectOffsetY) + strokeWidth/2"
                :width="rectDimension - strokeWidth"
                :height="rectDimension - strokeWidth"
                :fill="rectFillNotFinished"
              />
              <rect
                v-if="slot !== 'finished' && slot == 'running'"
                :y="0 + index_w*(rectDimension+rectOffsetX)"
                :x="0 + index*(rectDimension+rectOffsetY)"
                :width="rectDimensionForRunning(worker.runningProgress, worker.slots, index, false)"
                :height="rectDimension"
                :fill="rectFillFinished"
              />
            </g>
          </g>
        </svg>
      </div>
    </v-layout>
  </v-container>
</template>

<script lang="ts">
import { Component, Prop, Vue, Watch } from "vue-property-decorator";
import { mapActions, mapGetters } from "vuex";
import { WorkerDTO } from "../../dto/Worker";

@Component({
  components: {},
  props: ["matrixData", "isCollect2", "numOfSlotsPerWorker"],
})
export default class Matrix extends Vue {
  @Prop()
  public matrixData!: WorkerDTO[];
  public isCollect2!: boolean;
  public numOfSlotsPerWorker!: number;
  private pHeight: number = 1020;
  private pStrokeWidth = 1;
  private pFinishedColor = "#009975";
  private pRunningColor = "#ffffff";
  private pCollect2FirstStepSizeFactor = 5;

  get strokeWidth(): number {
    return this.pStrokeWidth;
  }

  get height(): number {
    if (this.viewOrientation === "horizontal") {
      return (
        this.matrixData.length * this.rectDimension +
        this.matrixData.length * this.rectOffsetY * 2 +
        this.boxOffset
      );
    } else {
      if (this.isCollect2) {
        return (
          this.numOfSlotsPerWorker * this.rectDimension +
          this.numOfSlotsPerWorker * this.rectOffsetY * 5 +
          this.boxOffset
        );
      } else {
        return (
          this.matrixData[0].slots.length * this.rectDimension +
          this.matrixData[0].slots.length * this.rectOffsetY * 2 +
          this.boxOffset
        );
      }
    }
  }

  get viewOrientation(): string {
    return this.$store.getters.flipMode;
  }

  get width(): number {
    return 1020;
  }

  get boxOffset(): number {
    return 12 + this.matrixData[0].slots.length * this.rectOffsetY;
  }

  get rectDimension(): number {
    return 12;
  }

  get rectOffsetX(): number {
    return 3;
  }

  get rectOffsetY(): number {
    return 2;
  }

  get viewBox(): string {
    return "0 0 " + this.width + " " + this.height;
  }
  get paddingBottomStyle(): string {
    return "padding-bottom: " + this.height + "px";
  }

  get rectFillFinished(): string {
    return this.pFinishedColor;
  }

  get rectFillNotFinished(): string {
    return this.pRunningColor;
  }

  get collect2FirstStepSizeFactor(): number {
    return this.pCollect2FirstStepSizeFactor;
  }

  public rectDimensionForRunning(
    progress: number,
    slots: string[],
    index: number,
    collect2: boolean,
  ): number {
    if (this.viewOrientation === "horizontal") {
      if (collect2 && slots[1] !== "finished") {
        return this.rectDimension;
      }

      if (index > 0 && slots[index - 1] !== "finished" && !collect2) {
        return 0;
      }
      const coeff: number = progress / 100;

      return this.rectDimension * coeff;
    } else {
      if (collect2 && slots[1] !== "finished") {
        return this.rectDimension;
      }

      if (index > 0 && slots[index - 1] !== "finished" && !collect2) {
        return this.rectDimension;
      }
      const coeff: number = progress / 100;
      return this.rectDimension - this.rectDimension * coeff;
    }
  }
  protected mounted() {
    this.$store.dispatch("loadFlipMode");
  }
}
</script>

<style scoped>
.svg_matrix_container {
  display: inline-block;
  position: relative;
  width: 100%;
  vertical-align: top;
  overflow: hidden;
}
.svg_matrix {
  display: inline-block;
  position: absolute;
  top: 0;
  left: 0;
}
</style>
