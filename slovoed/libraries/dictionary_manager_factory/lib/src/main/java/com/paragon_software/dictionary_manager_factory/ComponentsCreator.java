package com.paragon_software.dictionary_manager_factory;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.List;

import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.components.MorphoComponent;
import com.paragon_software.dictionary_manager.components.PictureComponent;
import com.paragon_software.dictionary_manager.components.SoundComponent;
import com.paragon_software.dictionary_manager.components.WordBaseComponent;

public class ComponentsCreator
{

  private static final List<DictionaryComponent> componentsHeap = new ArrayList<>();

  public List< DictionaryComponent > create( Product product )
  {
    List< DictionaryComponent > productComponents = new ArrayList<>();
    Components productComp = product.getComponents();

    addWordBaseComponent(productComponents, productComp.getDict(), false);
    addWordBaseComponent(productComponents, productComp.getDemoDict(), true);

    addSoundComponent(productComponents, productComp.getSound1(), false);
    addSoundComponent(productComponents, productComp.getDemoSound1(), true);
    addSoundComponent(productComponents, productComp.getSound2(), false);
    addSoundComponent(productComponents, productComp.getDemoSound2(), true);
    addSoundComponent(productComponents, productComp.getSound3(), false);
    addSoundComponent(productComponents, productComp.getDemoSound3(), true);
    addSoundComponent(productComponents, productComp.getSound4(), false);
    addSoundComponent(productComponents, productComp.getDemoSound4(), true);
    addSoundComponent(productComponents, productComp.getSound5(), false);
    addSoundComponent(productComponents, productComp.getDemoSound5(), true);
    addSoundComponent(productComponents, productComp.getSound6(), false);
    addSoundComponent(productComponents, productComp.getDemoSound6(), true);

    addMorphoComponent(productComponents, productComp.getMorpho1());
    addMorphoComponent(productComponents, productComp.getMorpho2());

    addPictBaseComponent(productComponents, productComp.getPict(), false);
    addPictBaseComponent(productComponents, productComp.getDemoPict(), true);

    return productComponents;
  }

  private void addWordBaseComponent( @NonNull List< DictionaryComponent > c, Dict d, boolean demo )
  {
    if ( null != d )
    {
      c.add(getWordBaseComponent(d, demo));
    }
  }

  private void addSoundComponent( @NonNull List< DictionaryComponent > components, Sound s, boolean demo )
  {
    if ( null != s )
    {
      components.add(getSoundComponent(s, demo));
    }
  }

  private void addMorphoComponent( @NonNull List< DictionaryComponent > components, Morpho m )
  {
    if ( null != m )
    {
      components.add(getMorphoComponent(m));
    }
  }

  private void addPictBaseComponent( @NonNull List< DictionaryComponent > c, Pict d, boolean demo )
  {
    if ( null != d )
    {
      c.add(getPictBaseComponent(d, demo));
    }
  }

  @NonNull
  private WordBaseComponent getWordBaseComponent( Dict d, boolean demo )
  {
    WordBaseComponent newComponent = new WordBaseComponent(d.getUrl(), Long.parseLong(d.getSize()), demo,
                                                           d.getVersion(), d.getLangFromShort(),
                                                           d.getLangToShort(), d.getPrcId(), d.getWordsCount());
    if ( !componentsHeap.contains(newComponent) )
    {
      componentsHeap.add(newComponent);
      return newComponent;
    }
    return (WordBaseComponent) componentsHeap.get(componentsHeap.indexOf(newComponent));
  }

  @NonNull
  private MorphoComponent getMorphoComponent( Morpho m )
  {
    MorphoComponent newComponent = new MorphoComponent(m.getId(), m.getUrl(), Long.parseLong(m.getSize()), m.getLang(), m.getSdcId());
    if ( !componentsHeap.contains(newComponent) )
    {
      componentsHeap.add(newComponent);
      return newComponent;
    }
    return (MorphoComponent) componentsHeap.get(componentsHeap.indexOf(newComponent));
  }

  @NonNull
  private SoundComponent getSoundComponent( Sound s, boolean demo )
  {
    SoundComponent newComponent = new SoundComponent(s.getUrl(), Long.parseLong(s.getSize()), demo, s.getVersion(),
                                                     s.getLangShort(), getSoundBaseName(s.getPrcId(), s.getLangIso()), s.getPrcId());
    if ( !componentsHeap.contains(newComponent) )
    {
      componentsHeap.add(newComponent);
      return newComponent;
    }
    return (SoundComponent) componentsHeap.get(componentsHeap.indexOf(newComponent));
  }

  @NonNull
  private PictureComponent getPictBaseComponent( Pict d, boolean demo )
  {
    PictureComponent newComponent = new PictureComponent(d.getUrl(), Long.parseLong(d.getSize()), demo,
                                                           d.getVersion(), d.getLangFromShort(), d.getPrcId());
    if ( !componentsHeap.contains(newComponent) )
    {
      componentsHeap.add(newComponent);
      return newComponent;
    }
    return (PictureComponent) componentsHeap.get(componentsHeap.indexOf(newComponent));
  }

  private String getSoundBaseName(String baseId, String langIso) {
    switch (baseId) {
      case "5608":
        return "British English spoken words";
      case "5609":
        return "American English spoken words";
      case "560B":
        return "British English spoken sentences";
      case "560C":
        return "American English spoken sentences";
      case "560A":
        return "World English spoken words";
      case "560D":
        return "World spoken sentences";
      default:
        return langIso;
    }
  }
}
