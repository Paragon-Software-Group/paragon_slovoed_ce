package com.paragon_software.information_manager;

import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.paragon_software.information_manager_ui.R;
import com.paragon_software.utils_slovoed.collections.CollectionView;

import java.util.LinkedHashMap;
import java.util.Map;

public class InformationFragment extends Fragment
{
  private static final String CONTROLLER_NAME = InformationFragment.class.getCanonicalName() + ".CONTROLLER_NAME";
  private InformationControllerAPI       controller;
  @Nullable
  private InformationRecyclerViewAdapter mInformationAdapter;

  private Map< InformationItemType, String > createTypes()
  {
    Map< InformationItemType, String > types = new LinkedHashMap<>();
    types.put(InformationItemType.PRIVACY_POLICY, getString(R.string.utils_slovoed_ui_privacy_policy));
    types.put(InformationItemType.EULA, getString(R.string.utils_slovoed_ui_eula));
    types.put(InformationItemType.ABOUT, getString(R.string.utils_slovoed_ui_about));
    types.put(InformationItemType.ADDITIONAL_ARTICLES, null);
    return types;
  }

  @Override
  public void onCreate( @Nullable Bundle savedInstanceState )
  {
    super.onCreate(savedInstanceState);
    InformationManagerAPI manager = InformationMangerHolder.getManager();
    controller = manager.getController(CONTROLLER_NAME);
  }

  @Override
  public void onDestroyView()
  {
    if ( null != mInformationAdapter )
    {
      mInformationAdapter.unregisterCollectionViewListener();
    }
    super.onDestroyView();
  }

  @Nullable
  @Override
  public View onCreateView( @NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                            @Nullable Bundle savedInstanceState )
  {
    View view = inflater.inflate(R.layout.fragment_information, container, false);
    RecyclerView mInformationRecyclerView = view.findViewById(R.id.information_list);
    mInformationRecyclerView.setLayoutManager(new LinearLayoutManager(getContext()));
    if ( null != controller )
    {
      mInformationAdapter =
          new InformationRecyclerViewAdapter(controller.getInformationItems(createTypes()), controller);
      mInformationAdapter.registerCollectionViewListener();
      mInformationRecyclerView.setAdapter(mInformationAdapter);
    }
    return view;
  }

  private static class InformationViewHolder extends RecyclerView.ViewHolder
  {
    TextView title;

    InformationViewHolder( View itemView )
    {
      super(itemView);
      title = itemView.findViewById(android.R.id.text1);
    }
  }

  private static class InformationRecyclerViewAdapter extends RecyclerView.Adapter< InformationViewHolder >
      implements CollectionView.OnItemRangeChanged
  {

    private final CollectionView< ? extends InformationItem, Void > informationItems;
    private final InformationControllerAPI                          controller;

    InformationRecyclerViewAdapter( CollectionView< ? extends InformationItem, Void > informationItems,
                                    InformationControllerAPI controller )
    {

      this.informationItems = informationItems;
      this.controller = controller;
    }

    @NonNull
    @Override
    public InformationViewHolder onCreateViewHolder( @NonNull ViewGroup parent, int viewType )
    {
      View convertView =
          LayoutInflater.from(parent.getContext()).inflate(android.R.layout.simple_list_item_1, parent, false);
      return new InformationViewHolder(convertView);
    }

    @Override
    public void onBindViewHolder( @NonNull InformationViewHolder holder, int position )
    {
      final InformationItem item = informationItems.getItem(position);
      if ( null != item )
      {
        holder.title.setText(item.getText());
        holder.title.setOnClickListener(new View.OnClickListener()
        {
          @Override
          public void onClick( View view )
          {
            if ( null != controller )
            {
              controller.openInformationItem(view.getContext(), item);
            }
          }
        });
      }
    }

    @Override
    public int getItemCount()
    {
      return informationItems.getCount();
    }

    void registerCollectionViewListener()
    {
      informationItems.registerListener(this);
    }

    void unregisterCollectionViewListener()
    {
      informationItems.unregisterListener(this);
    }

    @Override
    public void onItemRangeChanged( CollectionView.OPERATION_TYPE type, int startPosition, int itemCount )
    {
      notifyDataSetChanged();
    }
  }
}
