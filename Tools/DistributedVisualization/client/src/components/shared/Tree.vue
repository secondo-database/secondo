<template>
  <v-container grid-list-xl>
    <v-layout row wrap align-center>
      <div id="svg_container" class="svg_chart_container" :style="paddingBottomStyle">
        <svg
          class="svg_chart"
          preserveAspectRatio="xMaxYMax meet"
          :viewBox="viewBox"
          xmlns="http://www.w3.org/2000/svg"
          xmlns:xlink="http://www.w3.org/1999/xlink"
        >
          <g v-for="d in treeDescendants" :key="d.id" transform="translate(510 25)">
            <line
              v-for="child in d.children"
              :key="child.id"
              :x1="d.x"
              :y1="d.y"
              :x2="child.x"
              :y2="child.y"
              stroke="#313848"
              :stroke-width="strokeWidth"
            ></line>
            <rect :x="d.x-40" :y="d.y-12" width="75" height="25" fill="#FDFDFD"></rect>
            <text
              v-if="d.data.nowWorkingOn === true"
              :x="d.x"
              :y="d.y"
              class="now-working-on-node"
              dominant-baseline="middle"
              text-anchor="middle"
            >{{d.data.operator}}* {{d.data.result ? "("+d.data.result+")" : ""}}</text>
            <text
              v-else-if="d.data.ready === true && d.data.operand !== true"
              :x="d.x"
              :y="d.y"
              class="ready-node default"
              dominant-baseline="middle"
              text-anchor="middle"
            >{{d.data.operator}}  {{d.data.result ? "("+d.data.result+")" : ""}}</text>
            <text
              v-else-if="d.data.operand && !d.data.ready"
              :x="d.x"
              :y="d.y"
              class="operand default"
              dominant-baseline="middle"
              text-anchor="middle"
            >{{d.data.operator}}  {{d.data.result ? "("+d.data.result+")" : ""}}</text>
            <text
              v-else-if="d.data.operand && d.data.ready"
              :x="d.x"
              :y="d.y"
              class="operand ready default"
              dominant-baseline="middle"
              text-anchor="middle"
            >{{d.data.operator}}</text>
            <text
              v-else
              :x="d.x"
              :y="d.y"
              class="default"
              dominant-baseline="middle"
              text-anchor="middle"
            >{{d.data.operator}}  {{d.data.result ? "("+d.data.result+")" : ""}}</text>
          </g>
        </svg>
      </div>
    </v-layout>
  </v-container>
</template>

<script lang="ts">
import { Component, Prop, Vue, Watch } from "vue-property-decorator";
import { mapActions, mapGetters } from "vuex";
import * as d3 from "d3";

@Component({
  components: {},
  props: ["treeData"],
})
export default class Tree extends Vue {
  @Prop()
  public treeData!: object;

  private treeHierarchy!: d3.HierarchyNode<{}>;
  private treeLayout: d3.TreeLayout<{}> = d3.tree();
  private pHeight: number = 1020;

  public recalculateHeight(): void {
    let maxH: number = 0;
    for (const n of this.treeHierarchy.descendants() as any) {
      if (n.y > maxH) {
        maxH = n.y;
      }
    }
    this.pHeight = maxH + 5 * this.treeHierarchy.descendants().length + 30;
  }

  get paddingBottomStyle(): string {
    return "";
  }

  get strokeWidth(): number {
    return 2;
  }

  get height(): number {
    return this.pHeight;
  }

  get width(): number {
    return 1020;
  }

  get boxOffset(): number {
    return 50;
  }

  get viewBox(): string {
    return "0 0 " + this.width + " " + this.height;
  }

  get treeDescendants(): any {
    this.treeHierarchy = d3.hierarchy(this.treeData);
    this.treeLayout.nodeSize([80, 75]);
    this.treeLayout.separation((a, b) => {
      return a.parent === b.parent ? 1 : 3;
    });
    this.treeLayout(this.treeHierarchy);
    this.recalculateHeight();
    return this.treeHierarchy.descendants();
  }
}
</script>

<style scoped>
.default {
  font: 18px sans-serif;
  fill: #240041;
}
.now-working-on-node {
  font: bold italic 20px sans-serif;
  fill: #240041;
}
.ready-node {
  fill: #240041;
  text-decoration: line-through;
}
.operand {
  fill: #240041;
  font: italic 16px sans-serif;
}
.ready {
  text-decoration: line-through;
}
.svg_chart_container {
  display: flex;
  position: relative;
  width: 100%;
  vertical-align: top;
  overflow: hidden;
}
.svg_chart {
  display: inline-block;
  /* position: absolute; */
  top: 0px;
  left: 0;
}
</style>
