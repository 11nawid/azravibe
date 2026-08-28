import { BottomPanelKey, IdeGet, IdeSet, IdeState, PanelType, TabFile } from './types'

const panelPath = (panelType: PanelType) => `__panel__${panelType}`

export const createPanelTabToggle = (
  set: IdeSet,
  get: IdeGet,
  panelType: PanelType,
  title: string
) => {
  return (show: boolean) => {
    const path = panelPath(panelType)

    if (show) {
      const { tabs } = get()
      const existingTab = tabs.find((tab) => tab.isPanel && tab.panelType === panelType)
      if (existingTab) {
        set({ activePath: existingTab.path })
        return
      }

      const newTab: TabFile = {
        name: title,
        path,
        content: '',
        originalContent: '',
        isUnsaved: false,
        isPanel: true,
        panelType
      }

      set({
        tabs: [...tabs, newTab],
        activePath: newTab.path
      })
      return
    }

    const { tabs, activePath } = get()
    const remaining = tabs.filter((tab) => !(tab.isPanel && tab.panelType === panelType))
    const nextActive = activePath === path
      ? remaining[remaining.length - 1]?.path || null
      : activePath

    set({ tabs: remaining, activePath: nextActive })
  }
}

const panelVisibilityKey: Record<BottomPanelKey, keyof IdeState> = {
  terminal: 'terminalPanelVisible',
  problems: 'problemsPanelVisible',
  output: 'outputPanelVisible'
}

export const createBottomPanelVisibilityToggle = (
  set: IdeSet,
  get: IdeGet,
  panel: BottomPanelKey
) => {
  return (visible: boolean) => {
    const visibilityKey = panelVisibilityKey[panel]
    const { bottomPanelTabs } = get()

    if (visible) {
      const nextTabs = bottomPanelTabs.includes(panel)
        ? bottomPanelTabs
        : [...bottomPanelTabs, panel]

      set({
        bottomPanelTabs: nextTabs,
        activeBottomPanel: panel,
        problemsPanelOpen: true,
        [visibilityKey]: true
      } as Partial<IdeState>)
      return
    }

    const nextTabs = bottomPanelTabs.filter((tab) => tab !== panel)
    if (nextTabs.length === 0) {
      set({
        bottomPanelTabs: [],
        problemsPanelOpen: false,
        [visibilityKey]: false
      } as Partial<IdeState>)
      return
    }

    set({
      bottomPanelTabs: nextTabs,
      activeBottomPanel: nextTabs[0],
      [visibilityKey]: false
    } as Partial<IdeState>)
  }
}
