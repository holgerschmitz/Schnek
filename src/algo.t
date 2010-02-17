

template<class InputIterator>
typename InputIterator::value_type mean
  (
    InputIterator begin, 
    InputIterator end
  )
{
  typedef typename InputIterator::value_type Value;
  Value m(0);
  int cnt=0;
  for (InputIterator i=begin; i!=end; ++i)
  {
    m += (*i);
    ++cnt;
  }
  m /= double(cnt);
  return m;
}

