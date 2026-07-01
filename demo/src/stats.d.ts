declare module "stats.js" {
  export default class Stats {
    dom: HTMLElement;
    showPanel(index: number): void;
    begin(): void;
    end(): void;
  }
}
