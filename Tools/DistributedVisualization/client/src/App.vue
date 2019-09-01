<template>
  <v-app>
    <v-toolbar light fixed app>
      <v-toolbar-title>SECONDO</v-toolbar-title>
      <v-spacer></v-spacer>
      <div
        v-if="journalItems !== undefined && journalItems[0] !== undefined"
        class="pr-0 mr-0"
        wrap
        row
      >
        <v-chip
          :color="journalItems[0].status === 'running' ? 'indigo' : 'green'"
          text-color="white"
        >{{getElapsedTime(journalItems[0].started, journalItems[0].finished)}}</v-chip>

        <v-chip
          :color="journalItems[0].status === 'running' ? 'indigo' : 'green'"
          text-color="white"
        >{{journalItems[0].status}}</v-chip>

        <v-chip
          :color="journalItems[0].status === 'running' ? 'indigo' : 'green'"
          text-color="white"
        >
          <v-avatar
            :class="journalItems[0].status === 'running' ? 'indigo darken-4' : 'green darken-4'"
          >{{journalItems[0].numOfWorkers}}</v-avatar>workers
        </v-chip>
        <v-btn color="info" flat @click="flipView">Flip view</v-btn>
      </div>
    </v-toolbar>
    <v-content>
      <router-view />
    </v-content>
    <v-footer height="auto" light>
      <v-layout justify-center row wrap></v-layout>
    </v-footer>
  </v-app>
</template>

<script lang="ts">
import { Component, Prop, Vue } from "vue-property-decorator";
import { JournalDTO } from "./dto/Journal";
import { JournalService } from "./service/Journal";

@Component({
  components: {},
  props: {},
})
export default class App extends Vue {
  private clipped!: false;
  private drawer!: true;
  private fixed!: false;
  private miniVariant!: false;
  private title!: "SECONDO";

  public getElapsedTime(started: string, finished: string): string {
    return JournalService.calculateElapsedTime(started, finished);
  }

  public flipView() {
    this.$store.dispatch("switchFlipMode");
  }
  get journalItems(): JournalDTO[] {
    return this.$store.getters.journalItems;
  }
}
</script>
<style scoped>
.application {
  color: #211f26;
  background: #fdfdfd;
}
</style>
