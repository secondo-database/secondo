<template>
  <v-container grid-list-xl fluid>
    <v-layout row wrap class="my-1" align-center>
      <v-flex xs12>
        <v-alert v-model="errorAlert" dismissible type="error">{{error}}</v-alert>
        <span
          class="display-1"
          v-if="journalItems === undefined || journalItems.length === 0"
        >No running jobs</span>
        <div v-if="journalItems !== undefined && journalItems.length !== 0">
          <v-flex v-for="journalItem in journalItems" :key="journalItem.id" xs12 md12>
            <v-item-group>
              <v-container grid-list-xl fluid>
                <v-layout wrap>
                  <v-item>
                    <v-flex xs12 md12>
                      <v-card xs12 md12>
                        <v-card-title primary-title>
                          <div>
                            <!-- <div class="headline">{{journalItem.job_id}}</div> -->
                          </div>
                          <v-spacer></v-spacer>
                        </v-card-title>
                        <v-card-text>
                          <div class="font-weight-regular">
                            Entered query:
                            <span class="font-italic">{{journalItem.query}}</span>
                          </div>
                          <Tree :treeData="getTreeDataForJournalItem(journalItem)" />
                        </v-card-text>
                        <v-card-actions></v-card-actions>
                      </v-card>
                    </v-flex>
                  </v-item>
                  <v-item>
                    <v-flex xs12 md12>
                      <v-card>
                        <v-card-title primary-title>
                          <div>
                            <div class="title">Status of workers</div>
                          </div>
                        </v-card-title>
                        <v-card-text
                          v-if="runningWorkerStatus !== undefined && Object.keys(runningWorkerStatus).length !== 0"
                        >
                          <v-item-group>
                            <div v-for="k of (runningWorkerStatus)" :key="k.node">
                              <v-layout wrap>
                                <v-flex xs12 md12>
                                  <div>{{getSimplifiedResult(k.node)}}</div>
                                  <div>Operator: {{journalItem.tree[k.node].operator}}</div>
                                  <div>Arguments: {{getSimplifiedArgs(k.node)}}</div>
                                </v-flex>
                              </v-layout>
                              <v-layout align-left>
                                <Matrix
                                  v-if="journalItem.tree[k.node].operator === 'collect2'"
                                  :matrixData="getSlotsForJob(k.node).workers"
                                  :isCollect2="true"
                                  :numOfSlotsPerWorker="journalItem.numOfSlotsPerWorker"
                                />
                                <Matrix v-else :matrixData="getSlotsForJob(k.node).workers" />
                              </v-layout>
                            </div>
                          </v-item-group>
                          <v-card-actions></v-card-actions>
                        </v-card-text>
                        <v-card-text v-else>
                          <span class="font-weight-regular title font-italic">loading...</span>
                          <v-progress-linear :indeterminate="true"></v-progress-linear>
                        </v-card-text>
                      </v-card>
                    </v-flex>
                  </v-item>
                </v-layout>
              </v-container>
            </v-item-group>
          </v-flex>
        </div>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script lang="ts">
import { Component, Prop, Vue, Watch } from "vue-property-decorator";
import { setInterval, clearInterval } from "timers";
import { mapActions, mapGetters } from "vuex";
import { JournalDTO } from "../dto/Journal";
import * as moment from "moment";
import { WorkerStatusDTO } from "../dto/WorkerStatus";
import { RunningWorkerStatusDTO } from "../dto/RunningWorkerStatus";
import Tree from "./shared/Tree.vue";
import { TreeService } from "../service/Tree";
import axios, { AxiosResponse } from "axios";
import config from "../config/default";
import workerstatus from "../store/workerstatus";
import Matrix from "./shared/Matrix.vue";
import { LOAD_WORKERSTATUS_ONCE } from "../store/mutation_types";

@Component({
  components: {
    Tree,
    Matrix,
  },
})
export default class Journal extends Vue {
  private pollingJournal!: any;
  private pollingExtendedJournal!: any;
  private pollingWorkerStatus!: any;
  private pErrorAlert: boolean = false;
  private pTreeDialog: boolean = false;
  private pWorkerStatusRenderingKey: number = 0;

  public closeTreeDialog(): void {
    this.pTreeDialog = false;
  }

  public openTreeDialog(): void {
    this.pTreeDialog = true;
  }

  public getSlotsForJob(id: string): WorkerStatusDTO {
    return this.$store.getters.runningWorkerStatus.workerStatus[id];
  }

  public getColorForStatus(status: string): string {
    if (status === "finished") {
      return "green";
    } else {
      return "";
    }
  }

  public getFunctionForOperator(journal: JournalDTO, node: number): string {
    return TreeService.getArguments(journal, node.toString());
  }

  public getSimplifiedArgs(node: number): string {
    return TreeService.getSimplifiedArguments(node.toString());
  }

  public getSimplifiedResult(node: number): string {
    return TreeService.getSimplifiedResult(node.toString());
  }

  public getTreeDataForJournalItem(journalItem: JournalDTO): object {
    return this.$store.getters.treeForDisplay;
  }

  @Watch("error")
  protected onPropertyChangedError(newValue: string, oldValue: string) {
    if (newValue) {
      this.errorAlert = true;
    } else {
      this.errorAlert = false;
    }
  }

  get workerStatusRenderingKey(): number {
    return this.pWorkerStatusRenderingKey;
  }

  get treeDialog() {
    return this.pTreeDialog;
  }

  set treeDialog(value: boolean) {
    this.pTreeDialog = value;
  }

  get errorAlert(): boolean {
    if (this.error) {
      this.pErrorAlert = true;
    } else {
      this.pErrorAlert = false;
    }
    return this.pErrorAlert;
  }

  set errorAlert(value: boolean) {
    this.pErrorAlert = value;
  }

  get journalItems(): JournalDTO[] {
    return this.$store.getters.journalItems;
  }

  get runningWorkerStatus(): WorkerStatusDTO[] {
    const rws: RunningWorkerStatusDTO = this.$store.getters.runningWorkerStatus;
    const rwsReversed: RunningWorkerStatusDTO = { workerStatus: {} };
    const keys: string[] = Object.keys(rws.workerStatus).reverse();
    const ws: WorkerStatusDTO[] = [];
    for (const k of keys) {
      ws.push(rws.workerStatus[k]);
    }
    return ws;
  }

  get error(): any {
    return this.$store.getters.error;
  }

  protected mounted() {
    this.dispatchGetJournalAction();
    this.pollJournal();
    this.pollRunningJobWorkerStatus();
  }

  protected beforeDestroy() {
    clearInterval(this.pollingJournal);
    clearInterval(this.pollingWorkerStatus);
  }

  private pollJournal() {
    this.pollingJournal = setInterval(() => {
      this.dispatchGetJournalAction();
    }, 10000);
  }

  private pollRunningJobWorkerStatus() {
    this.pollingWorkerStatus = setInterval(() => {
      this.dispatchGetWorkerStatusAction();
    }, 1000);
  }

  private dispatchGetJournalAction(): void {
    this.$store.dispatch("getJournalAction");
  }

  private dispatchGetExtendedJournalAction(): void {
    this.$store.dispatch("getExtendedJournalAction");
  }

  private dispatchGetWorkerStatusAction(): void {
    this.$store.dispatch("pollWorkerStatusAction");
  }
}
</script>
